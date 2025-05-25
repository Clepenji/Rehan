//library calls

#include "IIS3DWB.h"
#include <SPI.h>
#include <WiFi.h>
//macros

#define SerialDebug true  // set to true to get Serial output for debugging
#define myLed    12
#define CSPIN    5

#define FIFO_WATERMARK       256

//IIS3DWB definitions
#define IIS3DWB_intPin1 34  // interrupt1 pin definitions, data ready
#define IIS3DWB_intPin2 35  // interrupt2 pin definitions, activity detection


//global variables

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
float AxyzData[FIFO_WATERMARK*2][3];                               // create FIFO data array
float Ax[FIFO_WATERMARK], Ay[FIFO_WATERMARK], Az[FIFO_WATERMARK];  // create accel data buffers
uint8_t fifo_mode = Contmode;
uint32_t start_dataRead, stop_dataRead;

float aRes;                              // scale resolutions per LSB for the accel
float accelBias[3] = {0.0f, 0.0f, 0.0f}; // offset biases for the accel
int16_t IIS3DWBData[4] = {0};            // Stores the 16-bit signed sensor output
float ax, ay, az, accelTemp;             // variables to hold latest accel data values
uint8_t IIS3DWBstatus;

volatile bool IIS3DWBDataReady = false, IIS3DWBFifoFilled = false;

IIS3DWB IIS3DWB(CSPIN); // instantiate IIS3DWB class




//wifi stuff
const char* ssid = "Phone1150";
const char* password = "hotspots";
const char* server_ip = "192.168.121.180"; // Replace with your server's IP
const int server_port = 12345;

WiFiClient client;


void setup() {
  //Comms initialisations
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

    IIS3DWB.initFIFO(FIFO_WATERMARK, fifo_mode, 0x0A); // use FIFO to collect data

    digitalWrite(myLed, HIGH); // turn off led when sensor configuration is finished
  }
  else
  {
    if (c != 0x6A) Serial.println(" IIS3DWB not functioning!");
    while (1) {};
  }



  attachInterrupt(IIS3DWB_intPin1, myinthandler1, RISING);   // define interrupt for intPin1 output of IIS3DWB
  attachInterrupt(IIS3DWB_intPin2, myinthandler2, FALLING);  // define interrupt for intPin2 output of IIS3DWB



  //Wifi initialisation

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




void loop() {
  // put your main code here, to run repeatedly:
  if (IIS3DWBDataReady) // FIFO threshold condition
  {
    IIS3DWBDataReady = false;

    fifo_count = IIS3DWB.FIFOstatus();        // read FIFO status
    if ( fifo_count >= FIFO_WATERMARK ) {     // if FIFO count valid

      start_dataRead = micros();                                                                                  // Diagnostic to track FFT calculation time; comment out when done
      IIS3DWB.readFIFOData(fifo_count, &AxyzData[0][0]);
      stop_dataRead = micros();                                                                                  // Diagnostic to track FFT calculation time; comment out when done
      Serial.print("SPI data read time: ");                                                                       // Diagnostic to track FFT calculation time; comment out when done
      Serial.print((stop_dataRead - start_dataRead)); Serial.print(" "); Serial.print("us");                       // Diagnostic to track FFT calculation time; comment out when done
      Serial.print(", FIFO Count: "); Serial.println(fifo_count);

      for (uint16_t ii = 0; ii < FIFO_WATERMARK; ii++)
      {
        Ax[ii] = AxyzData[ii][0] - accelBias[0]; // properly scale data in terms of gs
        Ay[ii] = AxyzData[ii][1] - accelBias[1];
        Az[ii] = AxyzData[ii][2] - accelBias[2];
      }

      if (client.connected()) {
        client.write((uint8_t*)Ax, fifo_count * sizeof(float));
        client.write((uint8_t*)Ay, fifo_count * sizeof(float));
        client.write((uint8_t*)Az, fifo_count * sizeof(float));
      } else {
        Serial.println("Disconnected. Trying to reconnect...");
        client.connect(server_ip, server_port);
      }
      delay(100);

      client.flush();
      Serial.println("Data is sent.");
      delay(2000);
      
      IIS3DWB.readFIFOData(FIFO_WATERMARK*2, &AxyzData[0][0]); //emptying buffer
    }
  }

  if (IIS3DWBFifoFilled) { // if activity change event FALLING detected
    IIS3DWBFifoFilled = false;

    if (SerialDebug) Serial.println("Fifo is full");
  } // end activity change interrupt handling

  // end sensor interrupt handling

//  IIS3DWBData[3] = IIS3DWB.readTempData(); // get IIS3DWB chip temperature
//  accelTemp = ((float) IIS3DWBData[3]) / 256.0f + 25.0f; // Accel chip temperature in degrees Centigrade
//  // Print temperature in degrees Centigrade
//  if (SerialDebug) {
//    Serial.print("IIS3DWB temperature is ");  Serial.print(accelTemp, 1);  Serial.println(" degrees C"); // Print T values to tenths of a degree C
//  }
//
//  digitalWrite(myLed, LOW); delay(10); digitalWrite(myLed, HIGH);   // toggle the led
}



void myinthandler1() {
  IIS3DWBDataReady = true;
}

void myinthandler2() {
  IIS3DWBFifoFilled = true;
}
