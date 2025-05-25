#include "MPU6050.h"

MPU6050 mpu;

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long currentMicros = 0;
unsigned long previousMicros = 0;

unsigned long time_diff = 0;

int16_t ax_raw, ay_raw, az_raw;


void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }

  uint8_t rateDivider = 1; // Example: Gyro Output Rate / (1 + 9) = 100 Hz
    mpu.setRate(rateDivider);

  mpu.CalibrateAccel(6);
  Serial.println("\nat 600 Readings");
  mpu.PrintActiveOffsets();



}

void loop() {
  currentMicros = micros();
  time_diff = currentMicros - previousMicros;

    // Read raw data from MPU6050
    
    if (time_diff >= 998) {
    mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);
    float ax = ax_raw / 16384.0;
    float ay = ay_raw / 16384.0;
    float az = az_raw / 16384.0;

    Serial.println(time_diff);

    previousMicros = currentMicros;
    } 
}#include <Wire.h>
#include "MPU6050.h"

MPU6050 mpu;


unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);// put your setup code here, to run once:
  Wire.begin();

  while (!Serial);

  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }

}

void loop() {
  int16_t ax_raw, ay_raw, az_raw;

  // Read raw data from MPU6050
  mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);
  float ax = ax_raw / 16384.0;
  float ay = ay_raw / 16384.0;
  float az = az_raw / 16384.0;

  Serial.print(ax);
  Serial.print("  ");
  Serial.print(ay);
  Serial.print("  ");
  Serial.print(az);
  Serial.print("  ");
  int m_time = millis() - previousMillis;
  Serial.print(m_time);
  Serial.print("millisecs   ");
  double herz = 1000.0 / m_time;
  Serial.print(herz);
  Serial.println("Hz");
  previousMillis = millis();

}
