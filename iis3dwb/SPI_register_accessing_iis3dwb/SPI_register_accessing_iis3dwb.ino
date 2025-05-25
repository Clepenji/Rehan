#include <IIS3DWB.h>
#include <SPI.h>


#define FIFOWATERMARK 256
#define CSPIN 5

// SPI pin definitions for ESP32 VSPI
uint8_t Ascale = AFS_2G;
uint8_t fifomode = Contmode;
uint16_t fifocount;
int16_t AxyzData[FIFOWATERMARK][3];
float Ax[FIFOWATERMARK], Ay[FIFOWATERMARK], Az[FIFOWATERMARK];
uint32_t startdataRead, stopdataRead;
float aRes;
float accelBias[3] = {0.0f, 0.0f, 0.0f};
int16_t IIS3DWBData[4] = {0};
float ax, ay, az, accelTemp;
volatile bool IIS3DWBDataReady = false, IIS3DWBWakeup = false;
IIS3DWB IIS3DWB(CSPIN);

void setup() {
  Serial.begin(115200);
  delay(4000);
  
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

    IIS3DWB.initFIFO(FIFOWATERMARK, fifomode);

    } else {
    Serial.println("IIS3DWB not functioning!");
    while (1);
  }

  attachInterrupt(IIS3DWBintPin1, myinthandler1, RISING);
  attachInterrupt(IIS3DWBintPin2, myinthandler2, FALLING);

  Serial.println("\nSPI Register R/W Ready");
  Serial.println("Write: W 0xAA 0x55");
  Serial.println("Read:  R 0xAA");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() == 0) return;

    char cmd = toupper(input.charAt(0));
    if (cmd == 'W') {
      // Write command: W 0xAA 0x55
      int firstSpace = input.indexOf(' ');
      int secondSpace = input.indexOf(' ', firstSpace + 1);
      if (firstSpace < 0 || secondSpace < 0) {
        Serial.println("Format: W 0xAA 0x55");
        return;
      }
      String regStr = input.substring(firstSpace + 1, secondSpace);
      String valStr = input.substring(secondSpace + 1);
      uint8_t reg = (uint8_t)strtol(regStr.c_str(), NULL, 16);
      uint8_t val = (uint8_t)strtol(valStr.c_str(), NULL, 16);

      digitalWrite(CSPIN, LOW);
      SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3)); // Mode 3 for IIS3DWB
      SPI.transfer(reg & 0x7F); // MSB=0 for write
      SPI.transfer(val);
      SPI.endTransaction();
      digitalWrite(CSPIN, HIGH);

      Serial.print("Wrote 0x");
      Serial.print(val, HEX);
      Serial.print(" to register 0x");
      Serial.println(reg, HEX);

    } else if (cmd == 'R') {
      // Read command: R 0xAA
      int spaceIdx = input.indexOf(' ');
      if (spaceIdx < 0) {
        Serial.println("Format: R 0xAA");
        return;
      }
      String regStr = input.substring(spaceIdx + 1);
      uint8_t reg = (uint8_t)strtol(regStr.c_str(), NULL, 16);

      digitalWrite(CSPIN, LOW);
      SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3)); // Mode 3 for IIS3DWB
      SPI.transfer(reg | 0x80); // MSB=1 for read
      uint8_t val = SPI.transfer(0x00); // Dummy byte to receive data
      SPI.endTransaction();
      digitalWrite(CSPIN, HIGH);

      Serial.print("Read register 0x");
      Serial.print(reg, HEX);
      Serial.print(": 0x");
      Serial.println(val, HEX);

    } else {
      Serial.println("Unknown command. Use W or R.");
    }
  }
}
