> [!NOTE] 
> ***Development of this project is continued at https://github.com/meshtastic/framework-portduino*** 

# Portduino

This is an attempt to port the Arduino API so that it can run on top of Linux (and other desktop operating systems).  This is to facilitate the following use-cases:

- You can run code originally intended to arduino like environments (nrf52, esp32, avr etc...) without modification as 'user-space' applications.  Those applications can talk to real SPI, I2C, GPIO, serial, wifi on your linux device using the 'standard' arduino APIs.  Many libraries/projects from platformio 'just work'.  Though of course the underlying OS is not an RTOS and hard-real-time constraints may get a bit fuzzy. ;-)
- You can debug/develop on a desktop OS where often the debugging environment is more forgiving and the compile/load/debug workflow is quite fast.
- You can run with all devices simulated (for automated integration testing or simulation) or some/all of the 'devices' connected to real hardware.

We've been using this project successfully on a fairly sizable & popular [Platformio](https://platformio.org/) project ([Meshtastic](https://github.com/meshtastic/)) for the last several months.  We use it both for our continuous integration tests (where we run our device software through crude simulated tests in github actions) and to support Meshtastic on linux for the Pine64 Lora USB dongle.

## Description

Someone wanted Meshtastic for a new linux based tablet, so we made a new new thing (which might be useful for other projects).

We implement the 'ArduinoCore' libs/API layer and support the following device level access from user-space regular apps:

- SPI (via simulation or the linux spidev device)
- Interrupt handlers (they won't know they are actually in userspace)
- GPIO control (via simulation or the linux gpio device)
- WiFi (via the arduino Wifi API, but via regular linux IPV4 functionality)
- Serial (but actually being done through any Unix file descriptor - so could be pipes/files/devices)
- I2C (not yet implemented in alpha, if you want this speak up)
- Any of the above can be implemented by particular 'Drivers' - so either the mainboard kernel-space SPI/I2C controller or via external USB to SPI/I2C/GPIO adapters

## How to use

This project is currently very 'alpha' mostly to judge community interest.  Once it is a bit more mature (if there is interest) I'm happy to send in pull-requests to 'platform-native' which add portuino as an optional framework.  If you would like to try it now, you should only need to add something like the following three lines
to your platformio.ini in the project you want to build.

```
platform = https://github.com/geeksville/platform-native.git 
build_flags = ${env.build_flags} -O0 -lgpiod 
framework = arduino
```

For an example (sizable) project that is using this see [meshtastic-device](https://github.com/meshtastic/Meshtastic-device).  Every checkin to that project triggers a new build of the 'native/portuino' version of meshtastic and runs it as a simulation test.  The github [actions](https://github.com/meshtastic/Meshtastic-device/actions) show the commands used to build the linux binary and the console output from running it.

## TODO (short term)

- Make a small users guide
- Currently only tested on Linux, generalize to make sure it works on Windows and MacOS (simulation only for those platforms initially)
- Make a tutorial (a tiny rasberry pi example that blinks a LED)
- Show that same example blinking a sim LED
- Add better command line processing
- Implement Thread and Lock
- Add platform-arm
- Improve InterruptHandler via threads
- Add I2C support (once someone wants it)

## TODO (long term)

- Change PORTDUINO def to PORTDUINO and PORTDUINO_Linux_x86_64
- Figure out why -Os breaks the executable and what part of platformio is setting that (it comes in before the platform stuff even)
- Split out threading library from meshtastic
- Add a scripting API for doing button presses, reading current screen contents, etc...
- Clean up C++ exception model
- Split out logging library
- Send in platformio PR
- Let LinuxSerial go places other than stdout
- Unify Thread/Semaphore/TypedQueue wrappers with the FreeRTOS versions

Copyright 2020 Geeksville Industries, LLC and Copyright (c) 2011-2014 Arduino LLC.  All right reserved. GPL LGPL license, see LICENSE file in this directory.
