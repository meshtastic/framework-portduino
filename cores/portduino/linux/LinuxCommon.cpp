//
// Created by kevinh on 9/1/20.
//

#include "Common.h"
#include "Utility.h"
#include "PortduinoGPIO.h"

#include <sched.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void delay(unsigned long milliSec) {
  //timespec ts{.tv_sec = (time_t)(milliSec / 1000),
  //            .tv_nsec = (long)(milliSec % 1000) * 1000L * 1000L};
  //nanosleep(&ts, NULL);
  if (realHardware)
    gpioIdle();
  usleep(milliSec * 1000); 
}

void delayMicroseconds(unsigned int usec) {
  usleep(usec); // better than nanosleep because it lets other threads run
}

void yield(void) { sched_yield(); }

#ifdef _WIN32
// libc's random(3) doesn't exist on Windows. Compose several rand() draws rather
// than forwarding to it directly: glibc's random() yields 31 bits while the
// Windows CRT's RAND_MAX is 0x7FFF (15 bits), and callers assume the wider range
// (apps draw nonces from this). Staying on rand() keeps the seeding contract
// identical to the Linux build, where randomSeed() calls srand() and glibc's
// srand() and srandom() are the same function.
long random(void)
{
    unsigned long v = (unsigned long)rand();
    v = (v << 15) ^ (unsigned long)rand();
    v = (v << 15) ^ (unsigned long)rand();
    return (long)(v & 0x7FFFFFFFUL);
}
#endif

long random(long max) { return random(0, max); }

long random(long min, long max) { 
  if (min >= max) {
    return min;
  }
  return rand() % (max - min) + min; 
}

void randomSeed(unsigned long s) { srand(s); }

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration)
    NOT_IMPLEMENTED("tone");

void noTone(uint8_t _pin) NOT_IMPLEMENTED("noTone");