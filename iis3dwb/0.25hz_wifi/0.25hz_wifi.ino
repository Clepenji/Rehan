{//Initialisations
#include <IIS3DWB.h>
#include <SPI.h>
#include <WiFi.h>

#include "esp_heap_caps.h"

#define CSPIN 5
#define IIS3DWBintPin1 34
#define IIS3DWBintPin2 35

#define FIFOWATERMARK 256

uint16_t currentDataBatch = 0;
float *AxyzData = (float *)heap_caps_malloc(FIFOWATERMARK * 3 * sizeof(float), MALLOC_CAP_SPIRAM);
float *Ax = (float *)heap_caps_malloc(131072 * sizeof(float), MALLOC_CAP_SPIRAM);
float *Ay = (float *)heap_caps_malloc(131072 * sizeof(float), MALLOC_CAP_SPIRAM);
float *Az = (float *)heap_caps_malloc(131072 * sizeof(float), MALLOC_CAP_SPIRAM);
int16_t IIS3DWBData[4] = {0};
float ax, ay, az, accelTemp;

/* Specify sensor parameters (sample rate is same as the bandwidth 6.3 kHz by default)
   choices are:  AFS_2G, AFS_4G, AFS_8G, AFS_16G
*/
uint8_t Ascale = AFS_2G;
uint8_t fifomode = Contmode;
float aRes;
float accelBias[3] = {0.0f, 0.0f, 0.0f};
volatile bool IIS3DWBDataReady = false, IIS3DWBFifoFilled = false;
IIS3DWB IIS3DWB(CSPIN);

//wifi stuff
const char* ssid = "Phone1150";
const char* password = "hotspots";
const char* server_ip = "192.168.121.180"; // Replace with your server's IP
const int server_port = 12345;

WiFiClient client;
}

void setup() {
  Serial.begin(115200);
  delay(4000);

  {//sensor iniitialisations
  SPI.begin();
  delay(20);
  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH);

  Serial.println("IIS3DWB accel...");

  uint8_t c = IIS3DWB.getChipID();
  Serial.print("IIS3DWB I AM ");
  Serial.print(c, HEX);
  Serial.print(" I should be ");
  Serial.println(0x7B, HEX);

  if (c == 0x7B) {
    Serial.println("IIS3DWB is online...");
    Serial.println("reset IIS3DWB to start fresh");
    IIS3DWB.reset();

    aRes = IIS3DWB.getAres(Ascale);
    IIS3DWB.selfTest();
    IIS3DWB.init(Ascale);
    IIS3DWB.offsetBias(accelBias);

    delay(1000);

    IIS3DWB.initFIFO(FIFOWATERMARK, fifomode, 0x0A);

    // Print CSV header
    Serial.println("Sample,X,Y,Z");
  } else {
    Serial.println("IIS3DWB not functioning!");
    while (1);
  }

  attachInterrupt(IIS3DWBintPin1, myinthandler1, RISING);
  attachInterrupt(IIS3DWBintPin2, myinthandler2, FALLING);
  }

  {//Wifi initialisation
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  Serial.print("Connecting to server...");
  while (!client.connect(server_ip, server_port)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to server!");
  }
  
  delay(5000);
}

void loop() {

  if (IIS3DWBFifoFilled) {
    IIS3DWB.readFIFOData(FIFOWATERMARK, AxyzData);
    IIS3DWBFifoFilled = false;

    for (uint16_t ii = 0; ii < FIFOWATERMARK; ii++) {
      Ax[ii + currentDataBatch] = AxyzData[ii * 3] - accelBias[0];
      Ay[ii + currentDataBatch] = AxyzData[ii * 3 + 1] - accelBias[1];
      Az[ii + currentDataBatch] = AxyzData[ii * 3 + 2] - accelBias[2];
    }

    currentDataBatch++;
  if (currentDataBatch == 512) {
    //Send data via wifi here
    //Ax, Ay, Az are the arrays
    //length of each array is 131072(2^17)
    //remember to flush the wifi
    
    currentDataBatch = 0;
    }
  }
  }
}


void myinthandler1() {
  IIS3DWBDataReady = true;
}

void myinthandler2() {
  IIS3DWBFifoFilled = true;
}
