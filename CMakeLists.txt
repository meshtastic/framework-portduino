cmake_minimum_required(VERSION 3.17)
project(Portduino VERSION 0.0.1 LANGUAGES C CXX)

add_library(
        Portduino
        cores/arduino/api/deprecated/Client.h
        cores/arduino/api/deprecated/HardwareSerial.h
        cores/arduino/api/deprecated/IPAddress.h
        cores/arduino/api/deprecated/Print.h
        cores/arduino/api/deprecated/Printable.h
        cores/arduino/api/deprecated/Server.h
        cores/arduino/api/deprecated/Stream.h
        cores/arduino/api/deprecated/Udp.h
        cores/arduino/api/deprecated/WString.h
        cores/arduino/api/deprecated-avr-comp/avr/dtostrf.h
        cores/arduino/api/deprecated-avr-comp/avr/interrupt.h
        cores/arduino/api/deprecated-avr-comp/avr/pgmspace.h
        cores/arduino/api/ArduinoAPI.h
        cores/arduino/api/Binary.h
        cores/arduino/api/Client.h
        cores/arduino/api/Common.cpp
        cores/arduino/api/Common.h
        cores/arduino/api/Compat.h
        cores/arduino/api/HardwareI2C.h
        cores/arduino/api/HardwareSerial.h
        cores/arduino/api/HardwareSPI.h
        cores/arduino/api/Interrupts.h
        cores/arduino/api/IPAddress.cpp
        cores/arduino/api/IPAddress.h
        cores/arduino/api/itoa.h
        # cores/arduino/api/PluggableUSB.cpp
        # cores/arduino/api/PluggableUSB.h
        cores/arduino/api/Print.cpp
        cores/arduino/api/Print.h
        cores/arduino/api/Printable.h
        cores/arduino/api/RingBuffer.h
        cores/arduino/api/Server.h
        cores/arduino/api/Stream.cpp
        cores/arduino/api/Stream.h
        cores/arduino/api/String.cpp
        cores/arduino/api/String.h
        cores/arduino/api/Udp.h
        cores/arduino/api/USBAPI.h
        cores/arduino/api/WCharacter.h
        cores/portduino/Arduino.h
        cores/portduino/itoa.cpp
        cores/portduino/dtostrf.c
        cores/portduino/linux/millis.cpp
        cores/portduino/simulated/SimCommon.cpp
        cores/portduino/Utility.cpp
        cores/portduino/Utility.h
        cores/portduino/simulated/SimHardwareSPI.cpp
        cores/portduino/simulated/SimHardwareI2C.cpp
        cores/portduino/simulated/SimHardwareI2C.h
        cores/portduino/linux/LinuxCommon.cpp
        cores/portduino/linux/LinuxSerial.cpp
        cores/portduino/linux/LinuxSerial.h
        cores/portduino/linux/LinuxHardwareI2C.cpp
        cores/portduino/linux/LinuxHardwareI2C.h)
set_target_properties(Portduino PROPERTIES PUBLIC_HEADER
        cores/arduino/api/ArduinoAPI.h
        cores/arduino/api/Binary.h
        cores/arduino/api/Client.h
        cores/arduino/api/Common.h
        cores/arduino/api/Compat.h
        cores/arduino/api/HardwareI2C.h
        cores/arduino/api/HardwareSerial.h
        cores/arduino/api/HardwareSPI.h
        cores/arduino/api/Interrupts.h
        cores/arduino/api/IPAddress.h
        cores/arduino/api/itoa.h
        cores/arduino/api/PluggableUSB.h
        cores/arduino/api/Printable.h
        cores/arduino/api/RingBuffer.h
        cores/arduino/api/Server.h
        cores/arduino/api/Stream.h
        cores/arduino/api/String.h
        cores/arduino/api/Udp.h
        cores/arduino/api/USBAPI.h
        cores/arduino/api/WCharacter.h
        cores/portduino/Arduino.h
        cores/portduino/FS/FS.h
        cores/portduino/FS/FSImpl.h
        cores/portduino/FS/PortduinoFS.h
        cores/portduino/FS/vfs_api.h
        cores/portduino/linux/gpio/LinuxGPIOPin.h
        cores/portduino/linux/LinuxHardwareI2C.h
        cores/portduino/linux/LinuxSerial.h
        cores/portduino/linux/PosixFile.h
        cores/portduino/logging.h
        cores/portduino/PortduinoGPIO.h
        cores/portduino/simulated/SimHardwareI2C.h
        cores/portduino/SPIChip.h
        cores/portduino/Utility.h
        cores/portduino/WString.h
)
target_include_directories(Portduino
        SYSTEM PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cores/arduino/api>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cores/portduino>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cores/portduino/FS>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
add_library(Portduino::Portduino ALIAS Portduino)

add_library(PortduinoMain
        cores/portduino/main.cpp
)
target_link_libraries(PortduinoMain PRIVATE Portduino)
add_library(Portduino::PortduinoMain ALIAS PortduinoMain)


install(
        TARGETS
        Portduino PortduinoMain
        EXPORT
        PortduinoTargets
        PUBLIC_HEADER
        CONFIGURATIONS Release
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/PortduinoConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/PortduinoConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/Portduino/cmake"
)
write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/PortduinoConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(
        EXPORT PortduinoTargets
        FILE PortduinoTargets.cmake
        NAMESPACE Portduino::
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/Portduino/cmake"
)

install(
        FILES
                "${PROJECT_BINARY_DIR}/PortduinoConfig.cmake"
                "${PROJECT_BINARY_DIR}/PortduinoConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/Portduino/cmake"
)
