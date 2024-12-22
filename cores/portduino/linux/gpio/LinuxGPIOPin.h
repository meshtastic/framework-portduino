//
// Created by kevinh on 9/1/20.
//

#ifdef PORTDUINO_LINUX_HARDWARE

#include "Arduino.h"
#include "Common.h"
#include "PortduinoGPIO.h"
#include "Utility.h"
#include "logging.h"

#include <assert.h>
#include <stdlib.h>
#include <gpiod.h>

#ifndef GPIOD_LINE_BULK_MAX_LINES
#define GPIOD_V 2
#define GPIOD_LINE_REQUEST_DIRECTION_AS_IS GPIOD_LINE_DIRECTION_AS_IS
#define gpiod_line gpiod_line_request
#else
#define GPIOD_V 1
#endif

/**
 * Adapts the modern linux GPIO API for use by Portduino
 */
class LinuxGPIOPin : public GPIOPin {
  /// Our GPIO line
  struct gpiod_line *line;

  /// Chip structure associated with the line
  struct gpiod_chip *chip;

public:

  /**
    * Create a pin given a linux chip label and pin name
    */
  LinuxGPIOPin(pin_size_t n, const char *chipLabel, const char *linuxPinName, const char *portduinoPinName = NULL);
  LinuxGPIOPin(pin_size_t n, const char *chipLabel, const int linuxPinNum, const char *portduinoPinName);

  /**
   * Constructor
   * @param l is the low level linux GPIO pin object
   */
  // LinuxGPIOPin(pin_size_t n, String _name, struct gpiod_line *l)
  //    : GPIOPin(n, _name), line(l) {}

  ~LinuxGPIOPin();

protected:
  /// Read the low level hardware for this pin
  virtual PinStatus readPinHardware();
  virtual void writePin(PinStatus s);
  virtual void setPinMode(PinMode m);
  unsigned int offset;
private:
  gpiod_line *getLine(const char *chipLabel, const int linuxPinNum);
  gpiod_line *getLine(const char *chipLabel, const char *linuxPinName);

  #if GPIOD_V == 2
  #define gpiod_line_release gpiod_line_request_release
  int gpiod_line_get_value(gpiod_line_request *line){return gpiod_line_request_get_value(line, offset);}
  int gpiod_line_set_value(gpiod_line_request *line, PinStatus s){return gpiod_line_request_set_value(line, offset, (gpiod_line_value) s);}
  #endif
};

#endif
