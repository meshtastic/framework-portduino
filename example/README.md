## Building the example

### Requirements

1. A system running Linux or Linux running in a VM.
2. gcc (`sudo apt-get install build-essential` or equivalent).
3. gpiod and libgpiod-dev library (`sudo apt-get install gpiod libgpiod-dev` or equivalent).
4. libbluetooth-dev library (`apt-get install libbluetooth-dev` or equivalent).
5. PlatformIO. See https:/platformio.org/install for instruction instructions.

Once all of the requirements have been met go into the example subdirectory
and run platform IO to build the example.

For example:

```
skip@Dell-7040:~../resources/framework-portduino$ cd example/
skip@Dell-7040:~../resources/framework-portduino/example$ git submodule init
Submodule 'libraries/WiFi' (https:/github.com/Meshtastic/WiFi.git) registered for path '../libraries/WiFi'
skip@Dell-7040:~../resources/framework-portduino/example$ git submodule update
Cloning into '/home/skip../resources/framework-portduino/libraries/WiFi'...
Submodule path '../libraries/WiFi': checked out 'b885b9595d54ee6eae59696eeae98f631eb27a23'
skip@Dell-7040:~../resources/framework-portduino/example$ pio run
Processing native (platform: https:/github.com/meshtastic/platform-native.git; framework: arduino; board: linux_hardware)
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Verbose mode can be enabled via `-v, --verbose` option
CONFIGURATION: https:/docs.platformio.org/page/boards/native/linux_hardware.html
PLATFORM: Native (1.2.1+sha.04435d0) > Portduino
HARDWARE: 95.37MB RAM, 95.37MB Flash
PACKAGES:
 - framework-portduino @ 0.0.1+sha.de3548f
LDF: Library Dependency Finder -> https:/bit.ly/configure-pio-ldf
LDF Modes: Finder ~ chain, Compatibility ~ soft
[nanopb] Installing Protocol Buffers dependencies
Requirement already satisfied: protobuf>=3.19.1 in /home/skip/.platformio/penv/lib/python3.8/site-packages (4.25.2)

... (lots of output deleted) ...

Compiling .pio/build/native/FrameworkArduino/portduino/simulated/SimHardwareSPI.cpp.o
Archiving .pio/build/native/libFrameworkArduino.a
Indexing .pio/build/native/libFrameworkArduino.a
Linking .pio/build/native/program
================================================================================== [SUCCESS] Took 1.99 seconds ==================================================================================
skip@Dell-7040:~../resources/framework-portduino/example$
```

