//
// Created by kevinh on 9/1/20.
//

#ifndef PORTDUINO_SIMHARDWAREI2C_H
#define PORTDUINO_SIMHARDWAREI2C_H

#include "HardwareI2C.h"
#include "Utility.h"

namespace arduino {

// result codes for endTransmission per Arduino docs
enum ResultI2c {
  I2cSuccess = 0,
  I2cTooLong,
  I2cAddrNAK,
  I2cDataNAK,
  I2cOtherError
};

class LinuxHardwareI2C : public HardwareI2C {
  int i2c_file;

public:
  void begin(const char* device);

  virtual void begin();

  virtual void begin(uint8_t address) {begin(); };

  virtual void end();

  virtual void setClock(uint32_t freq) NOT_IMPLEMENTED("i2csetClock");

  virtual void beginTransmission(uint8_t address);

  virtual uint8_t endTransmission(bool stopBit);

  virtual uint8_t endTransmission(void) { return endTransmission(true); }

  virtual uint8_t requestFrom(uint8_t address, size_t len, bool stopBit) {
    notImplemented("requestFrom");
    return 0;
  }

  virtual uint8_t requestFrom(uint8_t address, size_t len);

  virtual void onReceive(void (*)(int)) NOT_IMPLEMENTED("onReceive");

  virtual void onRequest(void (*)(void)) NOT_IMPLEMENTED("onRequest");

  // Methods from Print

  virtual size_t write(uint8_t toWrite);

  virtual size_t write(const uint8_t *buffer, size_t size);

  // Methods from Stream

  virtual int available();

  virtual int read();

  virtual uint8_t readBytes(uint8_t address, size_t len, bool stopBit) {
    notImplemented("requestFrom");
    return 0;
  }

  virtual size_t readBytes( char *buffer, size_t length);

  virtual int peek() {
    notImplemented("i2cpeek");
    return -1;
  }
};

extern LinuxHardwareI2C Wire;
} // namespace arduino

#endif // PORTDUINO_SIMHARDWAREI2C_H
