//
// Created by kevinh on 9/1/20.
//

#include "Utility.h"
#include <csignal>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#include <cstdlib>
#include <cstring>

// Declared in Arduino.h.

// Returns the length of src so callers can detect truncation, and always
// NUL-terminates when size > 0, matching the BSD contract.
extern "C" size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t len = strlen(src);
    if (size > 0) {
        size_t copy = len < size - 1 ? len : size - 1;
        memcpy(dst, src, copy);
        dst[copy] = '\0';
    }
    return len;
}

// _putenv_s always overwrites, so honour `overwrite == 0` explicitly.
extern "C" int setenv(const char *name, const char *value, int overwrite)
{
    if (!overwrite && getenv(name))
        return 0;
    return _putenv_s(name, value) == 0 ? 0 : -1;
}

// strcpy() returning a pointer to dst's terminating NUL rather than its start.
extern "C" char *stpcpy(char *dst, const char *src)
{
    size_t len = strlen(src);
    memcpy(dst, src, len + 1);
    return dst + len;
}
#endif // _WIN32

void notImplemented(const char *msg) { printf("%s is not implemented\n", msg); }

void portduinoError(const char *msg, ...) {
  char msgBuffer[256];
  va_list args;
  va_start(args, msg);
  vsnprintf(msgBuffer, sizeof msgBuffer, msg, args);
  va_end(args);
  printf("Portduino critical error: %s\n", msgBuffer);
  throw Exception(msgBuffer);
}

int portduinoCheckNotNeg(int result, const char *msg, ...) {
  if (result < 0) {
    printf("Portduino notneg errno=%d: %s\n", errno, msg);
    throw Exception(msg);
  }
  return result;
}


int portduinoCheckZero(int result, const char *msg, ...) {
  if (result != 0) {
    printf("Portduino checkzero %d: %s\n", result, msg);
    throw Exception(msg);
  }
  return result;
}

void portduinoDebug() {
  // Generate an interrupt
  std::raise(SIGINT);
}
