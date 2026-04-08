/*
 * Controlador SERIAL HC-SR04 - RMS Arduino Pong
 * Comunicação Digital - Zero Lag e Zero Tremedeira
 */

const int trigPin_C1 = 9;
const int echoPin_C1 = 10;
const int trigPin_C2 = 11;
const int echoPin_C2 = 12;

const int NUM_LEITURAS = 5; 
int leituras_C1[NUM_LEITURAS];
int leituras_C2[NUM_LEITURAS];
int indice = 0;
int total_C1 = 0;
int total_C2 = 0;

void setup() {
  pinMode(trigPin_C1, OUTPUT);
  pinMode(echoPin_C1, INPUT);
  pinMode(trigPin_C2, OUTPUT);
  pinMode(echoPin_C2, INPUT);

  // Inicia a comunicação Serial a 9600 baud
  Serial.begin(9600);

  for (int i = 0; i < NUM_LEITURAS; i++) {
    leituras_C1[i] = 0;
    leituras_C2[i] = 0;
  }
}

float lerDistanciaSegura(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 20000); 
  if (duration == 0) return 30.0; 
  return (duration * 0.0343) / 2.0;
}

void loop() {
  total_C1 = total_C1 - leituras_C1[indice];
  total_C2 = total_C2 - leituras_C2[indice];

  float dist_C1 = lerDistanciaSegura(trigPin_C1, echoPin_C1);
  float dist_C2 = lerDistanciaSegura(trigPin_C2, echoPin_C2);

  if (dist_C1 > 30.0) dist_C1 = 30.0;
  if (dist_C1 < 5.0) dist_C1 = 5.0;
  if (dist_C2 > 30.0) dist_C2 = 30.0;
  if (dist_C2 < 5.0) dist_C2 = 5.0;

  // Mapeia a distância diretamente para 0-127 (cabe em 1 byte)
  leituras_C1[indice] = map(dist_C1, 5, 30, 0, 127);
  leituras_C2[indice] = map(dist_C2, 5, 30, 0, 127);

  total_C1 = total_C1 + leituras_C1[indice];
  total_C2 = total_C2 + leituras_C2[indice];

  indice = indice + 1;
  if (indice >= NUM_LEITURAS) indice = 0;

  byte pos_P1 = total_C1 / NUM_LEITURAS;
  byte pos_P2 = total_C2 / NUM_LEITURAS;

  // Protocolo de Envio Serial (3 Bytes)
  Serial.write(255);    // Byte de Sincronia (Marcador de Início)
  Serial.write(pos_P1); // Posição Jogador 1 (0-127)
  Serial.write(pos_P2); // Posição Jogador 2 (0-127)

  delay(15); 
}