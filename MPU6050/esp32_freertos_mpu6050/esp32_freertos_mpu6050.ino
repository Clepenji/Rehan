#include <WiFi.h>
#include <WiFiClient.h>

#define ARR_SIZE 1

const char* ssid = "Phone1150";  // Replace with your WiFi SSID
const char* password = "hotspots";  // Replace with your WiFi Password
const char* server_ip = "172.16.46.18";  // Replace with your Python PC's local IP
const int server_port = 12345;

WiFiClient client;
#include "MPU6050.h"
double arr_x[250], arr_y[250], arr_z[250];

// WiFi credentials
const char* ssid = "Phone1150";
const char* password = "hotspots";
const char* server_ip = "192.168.105.180"; // Replace with your server's IP
const int server_port = 12345;

WiFiClient client;
MPU6050 mpu;


int16_t ax_raw, ay_raw, az_raw;
double ax[250], ay[250], az[250];





void wifiStack( void *pvParameters );
TaskHandle_t wifiStack_handle;
TaskHandle_t mpu_handle;


void setup() {
  Serial.begin(115200);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  xTaskCreatePinnedToCore(
    wifiStack, "send Data", 2048  // Stack size
    ,
    NULL  // When no parameter is used, simply pass NULL
    ,
    1  // Priority
    ,
    &wifiStack_handle  // With task handle we will be able to manipulate with this task.
    ,
    0  // Core on which the task will run
  );

  xTaskCreatePinnedToCore(
    read_mpu, "read mpu", 2048  // Stack size
    ,
    NULL  // When no parameter is used, simply pass NULL
    ,
    1  // Priority
    ,
    &mpu_handle  // With task handle we will be able to manipulate with this task.
    ,
    1  // Core on which the task will run
  );

  WiFi.begin(ssid, password);

  //Connecting to internet
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");



  //setting up inertial sensor
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





  //Connectiong to setup server
  Serial.print("Connecting to server...");
  while (!client.connect(server_ip, server_port)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to server!");
}

void loop() {
  ;
}


void wifiStack(void *pvParameters) {
  (void) pvParameters;

  while (1) {
    if (client.connected()) {
      client.write((uint8_t*)arr_x, sizeof(arr_x));
      client.write((uint8_t*)arr_y, sizeof(arr_y));
      client.write((uint8_t*)arr_z, sizeof(arr_z));
    } else {
      Serial.println("Disconnected. Trying to reconnect...");
      client.connect(server_ip, server_port);
    }

  }


}



void read_mpu(void *pvParameters) {
  (void) pvParameters;
  int number_of_samples_taken = 0;

  for (;;) {
    currentMicros = micros();
    
    if (currentMicros - previousMicros >= 998) {
      mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);
      arr_x[number_of_samples_taken] = ax_raw / 16384.0;
      arr_y[number_of_samples_taken] = ay_raw / 16384.0;
      arr_z[number_of_samples_taken] = az_raw / 16384.0;

      number_of_samples_taken += 1;
      if (number_of_samples_taken == 250) {            //test this with timeperiod check later
        vTaskSuspend();
      }

      previousMicros = currentMicros;
    }
  }
}
