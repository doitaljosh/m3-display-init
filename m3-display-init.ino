#include <Wire.h>

// I2C addresses of the FPDLink III serializer and deserializer chips.
// Both will always be on the same bus.
#define ADDR_SER 0x0c // DS90UB949
#define ADDR_DES 0x2c // DS90UB948 (inside Tesla display panel)

/*
 * @brief Set register regToWrite to value dataToWrite at address addrToWrite.
 * @addrToWrite 7-bit I2C address
 * @regToWrite 8-bit I2C register
 * @dataToWrite 8-bit register value
 */
int i2cSet(uint8_t addrToWrite, uint8_t regToWrite, uint8_t dataToWrite) {
  Wire.beginTransmission(addrToWrite);

  Wire.write(regToWrite);
  Wire.write(dataToWrite);

  Wire.endTransmission();

  return 0;
}

/*
 * @brief Returns a register value read from regToRead at address addrToRead.
 * @addrToRead 7-bit I2C address
 * @regToRead 8-bit I2C register
 * @return Register value read from regToRead
 */
uint8_t i2cGet(uint8_t addrToRead, uint8_t regToRead) {
  Wire.beginTransmission(addrToRead);

  Wire.write(regToRead);
  Wire.requestFrom(addrToRead, 1);
  uint8_t readBuf = Wire.read();

  Wire.endTransmission();

  return readBuf;
}

/*
 * @brief Attempts to write to an I2C register then reads it back to verify it has been written. A maximum of 3 attempts will be made to verify a write. 
 * @addrToWrite 7-bit I2C address
 * @regToWrite 8-bit I2C register
 * @dataToWrite 8-bit register value
 * @return Was write successful?
 */
bool i2cWriteReadCheck(uint8_t addrToWrite, uint8_t regToWrite, uint8_t dataToWrite) {
  int retry = 0;
  int i2cRetryMax = 3;

  while(retry++ < i2cRetryMax) {
    i2cSet(addrToWrite, regToWrite, dataToWrite);
    uint8_t i2cRead = i2cGet(addrToWrite, regToWrite);
    return (dataToWrite == i2cRead) ? 1 : 0;
  }
}

/*
 * @brief Read or write from the serializer
 * @direction 0=write, 1=read
 * @reg 8-bit register to read or write
 * @value Value to write
 * @return Result
 */
uint8_t serializer(int direction, uint8_t reg, uint8_t value) {
  switch(direction) {
    case 0:
      return i2cSet(ADDR_SER, reg, value);
    case 1:
      return i2cGet(ADDR_SER, reg);
    default:
      // We shouldn't get here
      return 0;
  }
}

/*
 * @brief Read or write from the deserializer
 * @direction 0=write, 1=read
 * @reg 8-bit register to read or write
 * @value Value to write
 * @return Result
 */
uint8_t deserializer(int direction, uint8_t reg, uint8_t value) {
  switch(direction) {
    case 0:
      return i2cSet(ADDR_DES, reg, value);
    case 1:
      return i2cGet(ADDR_DES, reg);
    default:
      // We shouldn't get here
      return 0;
  }
}

/*
 * @brief Set all serdes registers to the correct value for the Model 3 display to work. Taken directly from Model 3's firmware.
 */
int initDisplay() {
  // Local GPIO config
  serializer(0, 0x0d, 0x25); // Set GPIO0 as output with remote value
  serializer(0, 0x0f, 0x03); // Set GPIO3 as input for touch XRES

  // Local I2C config
  serializer(0, 0x17, 0x9e); // Enable forward-channel I2C passthrough

  delay(200);

  // Deserializer config
  deserializer(0, 0x41, 0x1f); // Set error count to max
  deserializer(0, 0x49, 0xe0); // Set output mode
  deserializer(0, 0x26, 0x0e); // Speed up I2C to 480KHz
  deserializer(0, 0x27, 0x0e);

  // Remote GPIO config
  deserializer(0, 0x1d, 0x13); // Set GPIO0 as input
  deserializer(0, 0x1e, 0x90); // Set GPIO2 high for backlight brightness
  deserializer(0, 0x1f, 0x05); // Set GPIO3 as output with remote value for touch XRES
  deserializer(0, 0x21, 0x91); // Set GPIO7/8 high for touch power/prox sense
  
  deserializer(0, 0x20, 0x09); // Set GPIO5 high for backlight enable

  serializer(0, 0x1e, 0x04); // Enable PORT1 I2C slave address
  return 0;
}

/*
 * @brief Set clocks, baud rate, and show init message.
 */
void setup() {
  Wire.begin();
  Wire.setClock(400000); // fast mode

  Serial.begin(115200); // serial for logging

  Serial.println("Init display start");
  initDisplay();
  Serial.println("Done");
}

/*
 * @brief Continuously check for presence of the serializer on the bus. If not present, print an error and retry after 1 second.
 */
void loop() {
  Wire.beginTransmission(ADDR_SER);
  if (Wire.endTransmission() != 0) {
    Serial.println("Serializer not detected. Reiniting display...");
    initDisplay();
  }
  delay(5000);
}
