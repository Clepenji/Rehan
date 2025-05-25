#include "IIS3DWB.h"
#include <SPI.h>

#define SerialDebug true  // set to true to get Serial output for debugging
#define myLed    12
#define CSPIN    5

#define FIFO_WATERMARK       256

//IIS3DWB definitions
#define IIS3DWB_intPin1 8  // interrupt1 pin definitions, data ready
#define IIS3DWB_intPin2 9  // interrupt2 pin definitions, activity detection

/* Specify sensor parameters (sample rate is same as the bandwidth 6.3 kHz by default)
   choices are:  AFS_2G, AFS_4G, AFS_8G, AFS_16G
*/
uint8_t Ascale = AFS_2G;
/*
  FIFO modes include: Bypassmode(FIFO disabled), FIFOmode (stops collecting when FIFO full),
   Cont_to_FIFOmode(Continuous mode until trigger deasserted, the FIFO mode),
   Bypass_to_Contmode (Bypass  mode until trigger deasserted, the Continuous mode),
   Contmode (If FIFO full, new sample overwrites older one),
   Bypass_to_FIFOmode (Bypass mode until trigger deasserted, the FIFO mode).

   FIFO size from 1 to 512 data samples.
*/
uint16_t fifo_count;                                               // FIFO buffer size variable
int16_t AxyzData[FIFO_WATERMARK][3];                               // create FIFO data array
float Ax[FIFO_WATERMARK], Ay[FIFO_WATERMARK], Az[FIFO_WATERMARK];  // create accel data buffers
uint8_t fifo_mode = Contmode;
uint32_t start_dataRead, stop_dataRead;

float aRes;                              // scale resolutions per LSB for the accel
float accelBias[3] = {0.0f, 0.0f, 0.0f}; // offset biases for the accel
int16_t IIS3DWBData[4] = {0};            // Stores the 16-bit signed sensor output
float ax, ay, az, accelTemp;             // variables to hold latest accel data values
uint8_t IIS3DWBstatus;

volatile bool IIS3DWB_DataReady = false, IIS3DWB_Wakeup = false;

IIS3DWB IIS3DWB(CSPIN); // instantiate IIS3DWB class






// SPI initialisations
#define HSPI_MISO 26
#define HSPI_MOSI 27
#define HSPI_SCLK 25
#define HSPI_SS   32

#if !defined(CONFIG_IDF_TARGET_ESP32)
#define VSPI FSPI
#endif

static const int spiClk = 1000000;  // 1 MHz

SPIClass *hspi = NULL;



void setup() {
  Serial.begin(115200);
  delay(4000);// put your setup code here, to run once:

  SPI.begin(); // Start SPI serial peripheral
  delay(20); // wait at least 10 ms for IIS3DWB boot procedure to complete
  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH); // disable SPI at start

  // Configure led
  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH); // start with led off

  // Read the IIS3DWB Chip ID register, this is a good test of communication
  Serial.println("IIS3DWB accel...");
  uint8_t c = IIS3DWB.getChipID();  // Read CHIP_ID register for IIS3DWB
  Serial.print("IIS3DWB "); Serial.print("I AM "); Serial.print(c, HEX); Serial.print(" I should be "); Serial.println(0x7B, HEX);
  Serial.println(" ");

  if (c == 0x7B) // check if all SPI sensors have acknowledged
  {
    Serial.println("IIS3DWB is online...");
    Serial.println(" ");

    // reset IIS3DWB to start fresh
    IIS3DWB.reset();

    digitalWrite(myLed, LOW); // indicate passed the ID check

    // get accel sensor resolution, only need to do this once
   aRes = IIS3DWB.getAres(Ascale);

   IIS3DWB.selfTest();

   IIS3DWB.init(Ascale); // configure IIS3DWB  

   IIS3DWB.offsetBias(accelBias);
   Serial.println("accel biases (mg)"); Serial.println(1000.0f * accelBias[0]); Serial.println(1000.0f * accelBias[1]); Serial.println(1000.0f * accelBias[2]);
   Serial.println(" ");
   delay(1000); 

   IIS3DWB.initFIFO(FIFO_WATERMARK, fifo_mode); // use FIFO to collect data
   
   digitalWrite(myLed, HIGH); // turn off led when sensor configuration is finished
  }
  else 
  {
  if(c != 0x6A) Serial.println(" IIS3DWB not functioning!"); 
  while(1){};
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
