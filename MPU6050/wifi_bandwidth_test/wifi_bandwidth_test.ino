#include <WiFi.h>
#include <WiFiClient.h>

const char* ssid = "Phone1150";
const char* password = "hotspots";
const char* server_ip = "192.168.105.180";  // Replace with receiver IP
const int server_port = 12345;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Test");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  

  while (!client.connect(server_ip, server_port)) {
    Serial.println("Connection failed.");
    return;
  }
  Serial.println("Connected to server.");
}

void loop() {
  for (int sec = 0; sec < 10; sec++) {
    for (int i = 0; i < 25; i++) {
      double arr1[250], arr2[250], arr3[250];
      for (int j = 0; j < 250; j++) {
        arr1[j] = random(1000) / 100.0;
        arr2[j] = random(1000) / 100.0;
        arr3[j] = random(1000) / 100.0;
      }

      if (client.connected()) {
        client.write((uint8_t*)arr1, sizeof(arr1));
        client.write((uint8_t*)arr2, sizeof(arr2));
        client.write((uint8_t*)arr3, sizeof(arr3));
        Serial.printf("Chunk %d of second %d sent\n", i + 1, sec + 1);
      } else {
        Serial.println("Disconnected. Trying to reconnect...");
        client.connect(server_ip, server_port);
      }

      delay(40);  // Wait 40 ms before next chunk
    }
  }

  client.stop();
  Serial.println("Done sending 10s of data at 1000Hz (in 25x250Hz chunks per second).");
  while (true);  // Stop loop
}
