// LIScribe – Firmware v3  (A1=Indice, A0=Medio, SDA=A4, SCL=A5)
// Libreria richiesta: MPU6050 by Electronic Cats

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// ── PIN FLEX ──────────────────────────────────
const int PINS[2] = {A1, A0};   // A1 = Indice, A0 = Medio

// Valori calibrati automaticamente in setup()
int DRITTO[2]  = {300, 300};    // ADC a mano aperta  (massimo)
int PIEGATO[2] = {260, 260};    // ADC a pugno chiuso (minimo)

// ── GIROSCOPIO ───────────────────────────────
float gx_off=0, gy_off=0, gz_off=0;
float angX=0,   angY=0,   angZ=0;
unsigned long lastT = 0;

// Doppio campione anti-crosstalk
int readFlex(int pin) {
  analogRead(pin); delay(8);
  analogRead(pin); delay(8);
  return analogRead(pin);
}

// ── CALIBRAZIONE GIROSCOPIO ──────────────────
void calibraGyro() {
  Serial.println("GYRO_CAL");
  long sx=0, sy=0, sz=0;
  int16_t ax,ay,az,gx,gy,gz;
  for (int i=0; i<150; i++) {
    mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);
    sx+=gx; sy+=gy; sz+=gz;
    delay(10);
  }
  gx_off = sx/150.0;
  gy_off = sy/150.0;
  gz_off = sz/150.0;
  angX = angY = angZ = 0;   // azzera angoli accumulati
  Serial.println("CALIBRATO");
}

// ── AUTO-CALIBRAZIONE FLEX ───────────────────
// Fase 1: mano aperta  (3s) → registra massimi → DRITTO
// Fase 2: pugno chiuso (3s) → registra minimi  → PIEGATO
void calibraFlex() {
  int maxV[2] = {0,    0};
  int minV[2] = {1023, 1023};

  Serial.println("FLEX_APRI");          // browser: "Apri la mano"
  delay(2500);                           // tempo per posizionare la mano
  unsigned long t = millis();
  while (millis() - t < 3000) {
    for (int i=0; i<2; i++) {
      int v = readFlex(PINS[i]);
      if (v > maxV[i]) maxV[i] = v;
    }
    delay(20);
  }

  Serial.println("FLEX_CHIUDI");        // browser: "Chiudi il pugno"
  delay(2500);                           // tempo per chiudere la mano
  t = millis();
  while (millis() - t < 3000) {
    for (int i=0; i<2; i++) {
      int v = readFlex(PINS[i]);
      if (v < minV[i]) minV[i] = v;
    }
    delay(20);
  }

  for (int i=0; i<2; i++) {
    DRITTO[i]  = maxV[i];
    PIEGATO[i] = minV[i];
    // Sicurezza: range minimo 8 punti ADC
    if (abs(DRITTO[i] - PIEGATO[i]) < 8) {
      DRITTO[i]  = 300;
      PIEGATO[i] = 260;
    }
  }
  Serial.println("FLEX_OK");
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("ERRORE_MPU");
    while(true);
  }

  delay(1000);
  calibraFlex();    // prima flex, poi gyro
  calibraGyro();
  lastT = millis();
  Serial.println("START");
}

void loop() {
  // Comandi da Python: R = ricalibra gyro, F = ricalibra flex
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'R') calibraGyro();
    if (cmd == 'F') calibraFlex();
  }

  int16_t ax,ay,az,gx,gy,gz;
  mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);

  // Flex → percentuale 0-100% con valori calibrati
  for (int i=0; i<2; i++) {
    int v   = readFlex(PINS[i]);
    int pct = constrain(map(v, DRITTO[i], PIEGATO[i], 0, 100), 0, 100);
    Serial.print(pct); Serial.print(",");
  }

  // Gyro → angoli integrati in gradi
  unsigned long now = millis();
  float dt = (now - lastT) / 1000.0;
  lastT = now;
  angX += (gx - gx_off) / 131.0 * dt;
  angY += (gy - gy_off) / 131.0 * dt;
  angZ += (gz - gz_off) / 131.0 * dt;

  Serial.print(angX,1); Serial.print(",");
  Serial.print(angY,1); Serial.print(",");
  Serial.println(angZ,1);

  delay(50);  // 20 Hz
}