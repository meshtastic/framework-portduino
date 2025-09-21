//
// Created by kevinh on 9/1/20.
//

#include "Common.h"
#include "Utility.h"
#include "logging.h"
#include "Arduino.h"
#include "PortduinoGPIO.h"
#include "gpiod.h"

#include <assert.h>
#include <stdlib.h>
#include <vector>

// #include "linux/gpio/classic/GPIOChip.h"

int NUM_GPIOS;

bool realHardware = false;

std::vector<std::unique_ptr<GPIOPinIf>> pins;

GPIOPinIf::~GPIOPinIf() {}

/** By default we assign simulated GPIOs to all pins, later applications can customize this in portduinoSetup */
void gpioInit(int _num_gpios) {
  NUM_GPIOS = _num_gpios;
  for(size_t i = 0; i < NUM_GPIOS; i++)
    pins.push_back(std::make_unique<SimGPIOPin>(i, "Unbound"));
}

void gpioIdle() {
  // log(SysGPIO, LogDebug, "doing idle refresh");
  for(size_t i = 0; i < NUM_GPIOS; i++)
    pins[i]->refreshIfNeeded();
}

void gpioBind(GPIOPinIf *p) {
  if (!(p->getPinNum() < NUM_GPIOS))
    log(SysGPIO, LogError, "getPinNum %u isn't smaller than max %d", p->getPinNum(), NUM_GPIOS);
  assert(p->getPinNum() < NUM_GPIOS);
  pins[p->getPinNum()].reset(p);
  realHardware = true;
}

/**
 * Return the specified GPIO pin or the UnboundPin pin instance */
GPIOPinIf *getGPIO(pin_size_t n)
{
  if (!(n < NUM_GPIOS))
    log(SysGPIO, LogError, "Pin %u isn't smaller than max %d", n, NUM_GPIOS);
  assert(n < NUM_GPIOS);
  return pins[n].get();
}

void pinMode(pin_size_t pinNumber, PinMode pinMode)
{
  // log(SysGPIO, LogDebug, "pinMode(%d, %d)", pinNumber, pinMode);
  auto p = getGPIO(pinNumber);
  // https://forums.raspberrypi.com/viewtopic.php?t=257773
  // https://docs.arduino.cc/learn/microcontrollers/digital-pins/
  p->setPinMode(pinMode);
}

void digitalWrite(pin_size_t pinNumber, PinStatus status)
{
  // log(SysGPIO, LogDebug, "digitalWrite(%d, %d)", pinNumber, status);
  auto p = getGPIO(pinNumber);
  p->writePin(status);
}

PinStatus digitalRead(pin_size_t pinNumber)
{
  auto p = getGPIO(pinNumber);
  auto r = p->readPin();

  // log(SysGPIO, LogDebug, "digitalRead(%d) -> %d", pinNumber, r);
  return r;
}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
  auto p = getGPIO(pin);
  auto r = p->pulseIn((PinStatus)state, timeout);

  // log(SysGPIO, LogDebug, "pulseIn(%d) -> %d", pinNumber, r);
  return r;
}

int analogRead(pin_size_t pinNumber)
{
  auto p = getGPIO(pinNumber);
  auto r = p->analogRead();

  log(SysGPIO, LogDebug, "analogRead(%d) -> %d", pinNumber, r);
  return r;
}

void analogWrite(pin_size_t pinNumber, int value)
{
  auto p = getGPIO(pinNumber);

  log(SysGPIO, LogDebug, "analogWrite(%d) -> %d", pinNumber, value);
  p->analogWrite(value);
}

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback,
                     PinStatus mode)
{
  //log(SysInterrupt, LogDebug, "attachInterrupt %d", interruptNumber);
  auto p = getGPIO(interruptNumber);
  p->attachInterrupt(callback, mode);
}

void detachInterrupt(pin_size_t interruptNumber)
{
  //log(SysInterrupt, LogDebug, "detachInterrupt %d", interruptNumber);
  auto p = getGPIO(interruptNumber);
  p->detachInterrupt();
}
