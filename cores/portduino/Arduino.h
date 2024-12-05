#pragma once

// Casts for Radiolib HAL
#define RADIOLIB_ARDUINOHAL_PIN_MODE_CAST (PinMode)
#define RADIOLIB_ARDUINOHAL_PIN_STATUS_CAST (PinStatus)
#define RADIOLIB_ARDUINOHAL_INTERRUPT_MODE_CAST (PinStatus)

#include "ArduinoAPI.h"
#include <argp.h>
#if defined(__AVR__)
#include "avr/pgmspace.h"
#else
#include "deprecated-avr-comp/avr/pgmspace.h"
#endif
#ifdef __cplusplus

#include "HardwareSPI.h"
#include "linux/LinuxSerial.h"
#include "linux/LinuxHardwareI2C.h"
#include <argp.h>

extern HardwareSPI SPI;

using namespace arduino;

typedef HardwareI2C TwoWire; // Some Arduino ports use this terminology

/** Map a pin number to an interrupt # 
 * We always map 1:1
*/
inline pin_size_t digitalPinToInterrupt(pin_size_t pinNumber) { return pinNumber; }

/** apps run under portduino can optionally define a portduinoSetup() to
 * use portduino specific init code (such as gpioBind) to setup portduino on
 * their host machine, before running 'arduino' code.
 * 
 * This function is called after portduinoCustomInit() (and after command line argument processing)
 */
extern void portduinoSetup();

/** Apps can optionally define this function to do *very* early app init.  typically you should just use it to call porduinoAddArguments()
 */
extern void portduinoCustomInit();

/**
 * call from portuinoCustomInit() if you want to add custom command line arguments
 */
void portduinoAddArguments(const struct argp_child &child, void *childArguments);

/**
 * write a 6 byte 'macaddr'/unique ID to the dmac parameter
 * This value can be customized with the --macaddr parameter and it defaults to 00:00:00:00:00:01
 */
void reboot();
#endif
