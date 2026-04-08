/********
 * ARDUINO PONG
 * Comunicação Serial (Zero Lag)
 * Sincronismo de Vídeo Corrigido para PAL / TV CFTV
 * Tempo baseado em Frames (Sem millis)
 ********/

#include <TVout.h>
#include <fontALL.h>

#define PADDLE_HEIGHT 14
#define PADDLE_WIDTH 1

#define RIGHT_PADDLE_X (TV.hres() - 4)
#define LEFT_PADDLE_X 2

#define IN_GAMEA 0
#define IN_GAMEB 0

#define IN_MENU 1
#define GAME_OVER 2

#define LEFT_SCORE_X (TV.hres() / 2 - 15)
#define RIGHT_SCORE_X (TV.hres() / 2 + 10)
#define SCORE_Y 9

#define MAX_Y_VELOCITY 6
#define PLAY_TO 7

#define LEFT 0
#define RIGHT 1

TVout TV;
unsigned char x, y;

// Posições vêm da Serial (0 a 127)
byte wheelOnePosition = 64;
byte wheelTwoPosition = 64;

int rightPaddleY = 0;
int leftPaddleY = 0;
unsigned char ballX = 0;
unsigned char ballY = 0;
char ballVolX = 2;
char ballVolY = 2;
int leftPlayerScore = 0;
int rightPlayerScore = 0;

int frame = 0;
int state = IN_MENU;

void processInputs() {
  // Lê a Serial de forma rápida e sem bloquear o vídeo
  while (Serial.available() >= 3) {
    if (Serial.read() == 255) {
      wheelOnePosition = Serial.read();
      wheelTwoPosition = Serial.read();
    }
  }
}

void showCountdown() {
  for (int i = 5; i > 0; i--) {
    TV.clear_screen();
    TV.select_font(font8x8);
    // Ajustado para o centro
    TV.set_cursor(60, 44);
    TV.print(i);

    // Beep curto
    TV.tone(800, 100);

    // Espera 1 segundo baseado em frames (PAL = 50 FPS)
    for (int f = 0; f < 50; f++) {
      processInputs();
      TV.delay_frame(1);
    }
  }

  TV.clear_screen();
  TV.select_font(font8x8);
  TV.set_cursor(52, 44);
  TV.print("GO!");

  // Beep longo de início
  TV.tone(1200, 500);

  // Espera meio segundo (25 frames)
  for (int f = 0; f < 25; f++) {
    processInputs();
    TV.delay_frame(1);
  }
}

void playGameOverMelody() {
  // Frequências (Hz) para um som clássico de derrota em Arcade
  int notas[] = { 330, 277, 233, 165 };

  // Duração de cada nota em frames (15 frames = 300ms em PAL 50FPS)
  // A última nota é mais longa (40 frames = 800ms)
  int duracaoFrames[] = { 15, 15, 15, 40 };

  for (int i = 0; i < 4; i++) {
    // Toca a nota no background
    TV.tone(notas[i], duracaoFrames[i] * 20);

    // Roda os frames para gastar o tempo da nota sem quebrar o vídeo
    for (int f = 0; f < duracaoFrames[i]; f++) {
      processInputs();
      TV.delay_frame(1);
    }
  }

  // Espera final de 2 segundos em silêncio (100 frames) para os jogadores verem o placar
  for (int f = 0; f < 100; f++) {
    processInputs();
    TV.delay_frame(1);
  }
}

void drawGameScreen() {
  rightPaddleY = (wheelOnePosition * (TV.vres() - PADDLE_HEIGHT)) / 127;
  x = RIGHT_PADDLE_X;
  for (int i = 0; i < PADDLE_WIDTH; i++) {
    TV.draw_line(x + i, rightPaddleY, x + i, rightPaddleY + PADDLE_HEIGHT, 1);
  }

  leftPaddleY = (wheelTwoPosition * (TV.vres() - PADDLE_HEIGHT)) / 127;
  x = LEFT_PADDLE_X;
  for (int i = 0; i < PADDLE_WIDTH; i++) {
    TV.draw_line(x + i, leftPaddleY, x + i, leftPaddleY + PADDLE_HEIGHT, 1);
  }

  TV.print_char(LEFT_SCORE_X, SCORE_Y, '0' + leftPlayerScore);
  TV.print_char(RIGHT_SCORE_X, SCORE_Y, '0' + rightPlayerScore);

  TV.set_pixel(ballX, ballY, 2);
}

void playerScored(byte player) {
  if (player == LEFT) leftPlayerScore++;
  if (player == RIGHT) rightPlayerScore++;

  if (leftPlayerScore == PLAY_TO || rightPlayerScore == PLAY_TO) {
    state = GAME_OVER;
  }
  ballVolX = -ballVolX;
}

void drawBox() {
  TV.clear_screen();
  TV.select_font(font4x6);
  TV.set_cursor(35, 2);
  TV.print("ARDUINO PONG");

  //Desenha rede
  for (int i = 1; i < TV.vres() - 4; i += 6) {
    TV.draw_line(TV.hres() / 2, i, TV.hres() / 2, i + 3, 1);
  }

  // Bordas adaptadas dinamicamente à resolução
  //TV.draw_line(0, 0, 0, TV.vres() - 1, 1);                          // left
  TV.draw_line(0, 0, TV.hres() - 1, 0, 1);                          // top
  //TV.draw_line(TV.hres() - 1, 0, TV.hres() - 1, TV.vres() - 1, 1);  // right
  TV.draw_line(0, TV.vres() - 1, TV.hres() - 1, TV.vres() - 1, 1);  // bottom

  state = IN_GAMEB;
}

void drawMenu() {
  x = 0;
  y = 0;
  char volX = 3;
  char volY = 3;
  TV.clear_screen();
  TV.select_font(font8x8);
  TV.set_cursor(16, 30);
  TV.print("ARDUINO PONG");
  TV.set_cursor(5, 42);
  TV.print("RIO MAKER SPACE");
  TV.set_cursor(16, 70);
  TV.print("PREPARE-SE !");

  // 4 segundos de animação = 200 frames em PAL (50 FPS)
  int framesRestantes = 200;

  while (framesRestantes > 0) {
    processInputs();
    TV.delay_frame(1);
    framesRestantes--;

    // Lógica da bolinha quicando no menu ajustada para TVout
    if (framesRestantes % 3 == 0) {
      if (x + volX < 1 || x + volX > TV.hres() - 1) volX = -volX;
      if (y + volY < 1 || y + volY > TV.vres() - 1) volY = -volY;

      TV.set_pixel(x, y, 0);  // Apaga a antiga
      x += volX;
      y += volY;
      TV.set_pixel(x, y, 1);  // Desenha a nova
    }
  }

  TV.select_font(font4x6);
}

void setup() {
  x = 0;
  y = 0;

  // INICIALIZAÇÃO CORRIGIDA PARA TV DE CFTV (PAL)
  // Se a tela ficar preta e branca rolando, troque _PAL por _NTSC
  TV.begin(_PAL, 128, 96);

  ballX = TV.hres() / 2;
  ballY = TV.vres() / 2;

  Serial.begin(9600);
}

void loop() {
  processInputs();

  if (state == IN_MENU) {
    drawMenu();
    showCountdown();
    state = IN_GAMEA;
  }

  if (state == IN_GAMEA) {
    drawBox();
  }

  if (state == IN_GAMEB) {
    if (frame % 3 == 0) {
      ballX += ballVolX;
      ballY += ballVolY;

      if (ballY <= 1 || ballY >= TV.vres() - 1) {
        ballVolY = -ballVolY;
        TV.tone(2000, 30);
      }

      if (ballVolX < 0 && ballX == LEFT_PADDLE_X + PADDLE_WIDTH - 1 && ballY >= leftPaddleY && ballY <= leftPaddleY + PADDLE_HEIGHT) {
        ballVolX = -ballVolX;
        ballVolY += 2 * ((ballY - leftPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
        TV.tone(2000, 30);
      }

      if (ballVolX > 0 && ballX == RIGHT_PADDLE_X && ballY >= rightPaddleY && ballY <= rightPaddleY + PADDLE_HEIGHT) {
        ballVolX = -ballVolX;
        ballVolY += 2 * ((ballY - rightPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
        TV.tone(2000, 30);
      }

      if (ballVolY > MAX_Y_VELOCITY) ballVolY = MAX_Y_VELOCITY;
      if (ballVolY < -MAX_Y_VELOCITY) ballVolY = -MAX_Y_VELOCITY;

      if (ballX <= 1) {
        playerScored(RIGHT);
        TV.tone(500, 300);
      }
      if (ballX >= TV.hres() - 1) {
        playerScored(LEFT);
        TV.tone(500, 300);
      }
    }

    drawGameScreen();
  }

  if (state == GAME_OVER) {
    drawGameScreen();
    TV.select_font(font8x8);
    TV.set_cursor(29, 25);
    TV.print("GAME");
    TV.set_cursor(68, 25);
    TV.print("OVER");

    TV.set_cursor(29, 35);
    TV.print("OBRIGADO");
    TV.set_cursor(29, 45);
    TV.print("POR JOGAR");

    // ADICIONE ESTA LINHA AQUI:
    playGameOverMelody();

    // Trava de 4 segundos usando frames (50 FPS * 4 = 200 frames)
    for (int f = 0; f < 200; f++) {
      processInputs();
      TV.delay_frame(1);
    }

    leftPlayerScore = 0;
    rightPlayerScore = 0;
    ballX = TV.hres() / 2;
    ballY = TV.vres() / 2;
    state = IN_MENU;
  }

  TV.delay_frame(1);
  // Em PAL a taxa de atualização é de 50 quadros por segundo
  if (++frame >= 50) frame = 0;
}