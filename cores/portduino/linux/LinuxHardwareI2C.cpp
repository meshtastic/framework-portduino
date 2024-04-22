//
// Created by kevinh on 9/1/20.
//
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "LinuxHardwareI2C.h"
#include <iostream>
#include <linux/i2c.h>

namespace arduino {
    char buf[100] = {0};
    int requestedBytes = 0;
    int sessionstatus;
    LinuxHardwareI2C Wire;
    bool hasBegun = false;

    void LinuxHardwareI2C::begin() {
        begin("/dev/i2c-1");
    }
    void LinuxHardwareI2C::begin(const char * device) {
        if (!hasBegun) {
            i2c_file = open(device, O_RDWR);
            hasBegun = true;
        }
    }
    void LinuxHardwareI2C::end() {
        if (hasBegun) {
            close(i2c_file);
            hasBegun = false;
        }
    }


    void LinuxHardwareI2C::beginTransmission(uint8_t address) {
        sessionstatus = ioctl(i2c_file, I2C_SLAVE, address);
    }
    uint8_t LinuxHardwareI2C::endTransmission(bool stopBit) {
        if (requestedBytes) {
            int resp = ::write(i2c_file, buf, requestedBytes);
            requestedBytes = 0;
            return resp;
        }
        return 0;

    }


    size_t LinuxHardwareI2C::write(uint8_t toWrite) {
        buf[requestedBytes] = toWrite; // todo: alloc and copy
        requestedBytes++;
        return 0;
    }
    size_t LinuxHardwareI2C::write(const uint8_t *buffer, size_t size) {
        return ::write(i2c_file, buffer, size); // Should this defer actual writing til transaction close?
    }

    int LinuxHardwareI2C::read() {
        int tmpBuf = 0;
        if (::read(i2c_file, &tmpBuf, 1) == -1)
            return -1;
        return tmpBuf;
    }

    size_t LinuxHardwareI2C::readBytes(char *buffer, size_t length) {
        int bytes_read = 0;

        if (length == 0) {
            return length;
        } else {
            bytes_read = ::read(i2c_file, buffer, length);
            if ( bytes_read < 0) bytes_read = 0;
        }
        return bytes_read;
    }

    int LinuxHardwareI2C::available() {
        int numBytes;
        ioctl(i2c_file, FIONREAD, &numBytes);
        return numBytes;
    }

    uint8_t LinuxHardwareI2C::requestFrom(uint8_t address, size_t) {
        return ioctl(i2c_file, I2C_SLAVE, address);
    }
}