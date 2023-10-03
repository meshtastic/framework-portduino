//
// Created by kevinh on 9/1/20.
//

#include "Common.h"
#include "Utility.h"

#include <sched.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void delay(unsigned long milliSec) {
  //timespec ts{.tv_sec = (time_t)(milliSec / 1000),
  //            .tv_nsec = (long)(milliSec % 1000) * 1000L * 1000L};
  //nanosleep(&ts, NULL);
  usleep(milliSec * 1000); 
}

void delayMicroseconds(unsigned int usec) {
  usleep(usec); // better than nanosleep because it lets other threads run
}

void yield(void) { sched_yield(); }

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