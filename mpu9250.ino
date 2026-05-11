#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

const int PINS[3]    = {A0, A1, A2};
const int DRITTO[3]  = {305, 309, 317};
const int PIEGATO[3] = {260, 278, 285};

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connesso!");
  } else {
    Serial.println("Errore: sensore non trovato.");
    while (true);
  }

  Serial.println("A2,A1,A0,GX,GY,GZ");
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  for (int i = 2; i >= 0; i--) {
    int flex = constrain(map(analogRead(PINS[i]), DRITTO[i], PIEGATO[i], 0, 100), 0, 100);
    Serial.print(flex);
    Serial.print(",");
  }

  Serial.print(gx / 131.0, 3); Serial.print(",");
  Serial.print(gy / 131.0, 3); Serial.print(",");
  Serial.println(gz / 131.0, 3);

  delay(200);
}