#include <WiFi.h>
#include <Wire.h>
#include "Bonezegei_DHT11.h"
#include "MPU6050.h"

// WiFi credentials
const char* ssid = "Phone1150";
const char* password = "hotspots";
const char* server_ip = "192.168.191.180"; // Replace with your server's IP
const int server_port = 12345;

WiFiClient client;
MPU6050 mpu;
Bonezegei_DHT11 dht(15);
float tempDeg = 0;

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

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
  currentMillis = millis();

  if (client.connected()) {
    int16_t ax_raw, ay_raw, az_raw;

    // Read raw data from MPU6050
    mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);
    float ax = ax_raw / 16384.0;
    float ay = ay_raw / 16384.0;
    float az = (az_raw / 16384.0) - 1;

    // Read temperature from DHT11
    if (currentMillis - previousMillis >= 500) {
      if (dht.getData()) {
        tempDeg = dht.getTemperature();
      }
      previousMillis = currentMillis;
    }

    // Format data as a CSV string with two-decimal precision
    String data = String(tempDeg, 2) + "," + String(ax, 2) + "," + String(ay, 2) + "," + String(az, 2) + "\n";

    // Send data to the server
    client.print(data);
    Serial.println("Sent: " + data);


    delay(200);  // Adjust delay as needed
  } else {
    Serial.println("Disconnected. Reconnecting...");
    client.connect(server_ip, server_port);
  }
}
