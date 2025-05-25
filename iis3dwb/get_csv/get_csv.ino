#include <IIS3DWB.h>
#include <SPI.h>
#include "esp_heap_caps.h"

//It is recommended to keep this number a power of 2(Eg. 1024 = 2^10)
#define NUMBER_OF_SAMPLES 131072 //This is 2^17

#define CSPIN 5
#define IIS3DWBintPin1 34
#define IIS3DWBintPin2 35

#define FIFOWATERMARK 256

uint16_t currentDataPoint = 0;
int16_t  AxyzData[4];

float* all_data = NULL;
uint8_t Ascale = AFS_2G;
uint8_t fifomode = Contmode;
float aRes;
float accelBias[3] = {0.0f, 0.0f, 0.0f};
volatile bool IIS3DWBDataReady = false, IIS3DWBFifoFilled = false;
IIS3DWB IIS3DWB(CSPIN);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  delay(1000);

  //PSRAM Initialisation
  if (psramInit()) {
    all_data = (float*)ps_calloc(131072 * 3, sizeof(float));
    if (!all_data) {
      Serial.println("PSRAM allocation failed!");
      while (1);
    }
  }
  delay(2000);
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

    //IIS3DWB.initFIFO(FIFOWATERMARK, fifomode, 0x0A);

    // Print CSV header
    Serial.println("Sample,X,Y,Z");
  } else {
    Serial.println("IIS3DWB not functioning!");
    while (1);
  }

  attachInterrupt(IIS3DWBintPin1, myinthandler1, RISING);
  attachInterrupt(IIS3DWBintPin2, myinthandler2, RISING);
  //IIS3DWB.readFIFOData(FIFOWATERMARK*2, &AxyzData[0][0]); //empty fifo before beginning
}


void loop() {
  if (IIS3DWBDataReady) {
    IIS3DWBDataReady = false;

    IIS3DWB.readAccelData(AxyzData);

    all_data[currentDataPoint*3] = (float)AxyzData[0]*aRes - accelBias[0];
    all_data[currentDataPoint*3 + 1] = (float)AxyzData[1]*aRes - accelBias[1];
    all_data[currentDataPoint*3 + 2] = (float)AxyzData[2]*aRes - accelBias[2];

    currentDataPoint++;
    if (currentDataPoint >= NUMBER_OF_SAMPLES) {
      // Print as CSV: Sample,X,Y,Z
      for (int ii = 0; ii < NUMBER_OF_SAMPLES; ii++) {
        Serial.print(ii+1);
        Serial.print(",");
        Serial.print(all_data[ii * 3], 6);
        Serial.print(",");
        Serial.print(all_data[ii * 3 + 1], 6);
        Serial.print(",");
        Serial.println(all_data[ii * 3 + 2], 6);
        delayMicroseconds(10);
      }

      currentDataPoint = 0;
      delay(6000);
    }
  }
}


void myinthandler1() {
  IIS3DWBDataReady = true;
}

void myinthandler2() {
  IIS3DWBFifoFilled = true;
}
