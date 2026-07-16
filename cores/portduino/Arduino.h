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

#ifdef _WIN32
// POSIX/BSD functions the Windows CRT lacks. Defined in Utility.cpp.
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
size_t strlcpy(char *dst, const char *src, size_t size);
int setenv(const char *name, const char *value, int overwrite);
char *stpcpy(char *dst, const char *src);
#ifdef __cplusplus
}
#endif
#endif // _WIN32
#ifdef __cplusplus

#include "HardwareSPI.h"
#include "linux/LinuxSerial.h"
#include "linux/LinuxHardwareI2C.h"

extern HardwareSPI SPI;

#ifdef _WIN32
#include <cstdio>
namespace arduino
{
// Windows is LLP64, so size_t is `unsigned long long` where LP64 Linux and macOS
// have `unsigned long`. ArduinoCore-API's String stops at long/unsigned long, so
// `someString + someSizeT` binds exactly on LP64 but is ambiguous here. Supply
// the missing 64-bit overloads instead of casting at each call site, which would
// truncate. Found by ADL; kept out of the vendored String.h to keep that tree
// close to upstream.
inline StringSumHelper &operator+(const StringSumHelper &lhs, unsigned long long num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    char buf[24];
    snprintf(buf, sizeof(buf), "%llu", num);
    a.concat(buf);
    return a;
}

inline StringSumHelper &operator+(const StringSumHelper &lhs, long long num)
{
    StringSumHelper &a = const_cast<StringSumHelper &>(lhs);
    char buf[24];
    snprintf(buf, sizeof(buf), "%lld", num);
    a.concat(buf);
    return a;
}
} // namespace arduino
#endif // _WIN32

using namespace arduino;

struct portduinoOptions {
    // char *fileSystemPath = nullptr;
    bool realHardware = false;
};

typedef HardwareI2C TwoWire; // Some Arduino ports use this terminology

/** Map a pin number to an interrupt #
 * We always map 1:1
*/
inline pin_size_t digitalPinToInterrupt(pin_size_t pinNumber) { return pinNumber; }

#ifdef _WIN32
/** Zero-argument random(). Common.h declares only random(max) / random(min, max);
 * the no-argument form resolves to libc's random(3) on Linux and macOS, which the
 * Windows CRT lacks. Defined in LinuxCommon.cpp.
 */
long random(void);
#endif

/** apps run under portduino can optionally define a portduinoSetup() to
 * use portduino specific init code (such as gpioBind) to setup portduino on
 * their host machine, before running 'arduino' code.
 * 
 * This function is called after portduinoCustomInit() (and after command line argument processing)
 */
extern void portduinoSetup();

/** Apps can optionally define this function to do *very* early app init.  typically you should just use it to call portduinoAddArguments()
 */
extern void portduinoCustomInit();

/**
 * call from portuinoCustomInit() if you want to add custom command line arguments
 */
void portduinoAddArguments(const struct argp_child &child, void *childArguments);

/**
 * This allows adding some custom options to how portduino operates.
 */
void portduinoSetOptions(portduinoOptions);

/**
 * write a 6 byte 'macaddr'/unique ID to the dmac parameter
 * This value can be customized with the --macaddr parameter and it defaults to 00:00:00:00:00:01
 */
void reboot();
#endif
