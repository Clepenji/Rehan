#include <WiFi.h>
#include <Wire.h>
#include "Bonezegei_DHT11.h"
#include "MPU6050.h"

// WiFi credentials
const char* ssid = "Phone1150";
const char* password = "hotspots";
const char* server_ip = "192.168.105.180"; // Replace with your server's IP
const int server_port = 12345;

WiFiClient client;
MPU6050 mpu;

int16_t ax_raw, ay_raw, az_raw;
float arr_x[1000], arr_y[1000], arr_z[1000];
Bonezegei_DHT11 dht(15);
float tempDeg = 0;

unsigned long currentMillis = 0;
unsigned long currentMicros = 0;
unsigned long previousMillis = 0;
unsigned long previousMicros = 0;



void setup() {
  Serial.begin(115200);
  Wire.begin();
  dht.begin();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");


  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }

  mpu.CalibrateAccel(6);
  Serial.println("\nat 600 Readings");
  mpu.PrintActiveOffsets();


  Serial.print("Connecting to server...");
  while (!client.connect(server_ip, server_port)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to server!");
}

void loop() {
  // initiate sammpling after 3 secs
  delay(3000);
  boolean sampling = true;
  int number_of_samples_taken = 0;


  // sample data and store it in an array of size 1000
  for(int i=0; i<1000; i++) {
    mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);
    arr_x[i] = (float)ax_raw / 16384.0f;
    arr_y[i] = (float)ay_raw / 16384.0f;
    arr_z[i] = (float)az_raw / 16384.0f;
    delayMicroseconds(1998);
  }

  Serial.println("Read all data.");



  // sampling is done. Now send the data
  //send data in chunks of 100 samples
  for (int i = 0; i < 4; i++) {
    if (client.connected()) {
      client.write((uint8_t*)&arr_x[i * 250], 250 * sizeof(float));
      client.write((uint8_t*)&arr_y[i * 250], 250 * sizeof(float));
      client.write((uint8_t*)&arr_z[i * 250], 250 * sizeof(float));
    } else {
      Serial.println("Disconnected. Trying to reconnect...");
      i--;
      client.connect(server_ip, server_port);
    }

    delay(40);
  }
  client.flush();
  
  Serial.println("Data is sent.");
}
