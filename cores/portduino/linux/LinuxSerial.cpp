//
// Created by kevinh on 9/1/20.
//

#include "LinuxSerial.h"
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <string>
#include <sys/ioctl.h>

#include <stdio.h>

struct termios tty;
namespace arduino {
    LinuxSerial Serial1;
    SimSerial Serial;
    // https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

    void LinuxSerial::begin(unsigned long baudrate, uint16_t config) {
        serial_port = open(path.c_str(), O_RDWR);
        tcgetattr(serial_port, &tty);

        tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
        tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
        tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
        tty.c_cflag |= CS8; // 8 bits per byte (most common)
        tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
        tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO; // Disable echo
        tty.c_lflag &= ~ECHOE; // Disable erasure
        tty.c_lflag &= ~ECHONL; // Disable new-line echo
        tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
        tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
        // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
        // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

        tty.c_cc[VTIME] = 0;    // don't wait
        tty.c_cc[VMIN] = 0;

        speed_t speed;
        switch(baudrate)
        {
#ifdef B1200
            case 1200:
                speed = B1200;
                break;
#endif
#ifdef B2400
            case 2400:
                speed = B2400;
                break;
#endif
#ifdef B4800
            case 4800:
                speed = B4800;
                break;
#endif
#ifdef B9600
            case 9600:
                speed = B9600;
                break;
#endif
#ifdef B19200
            case 19200:
                speed = B19200;
                break;
#endif
#ifdef B38400
            case 38400:
                speed = B38400;
                break;
#endif
#ifdef B57600
            case 57600:
                speed = B57600;
                break;
#endif
#ifdef B115200
            case 115200:
                speed = B115200;
                break;
#endif
#ifdef B230400
            case 230400:
                speed = B230400;
                break;
#endif
#ifdef B460800
            case 460800:
                speed = B460800;
                break;
#endif
#ifdef B500000
            case 500000:
                speed = B500000;
                break;
#endif
#ifdef B576000
            case 576000:
                speed = B576000;
                break;
#endif
#ifdef B921600
            case 921600:
                speed = B921600;
                break;
#endif
#ifdef B1000000
            case 1000000:
                speed = B1000000;
                break;
#endif
#ifdef B1152000
            case 1152000:
                speed = B1152000;
                break;
#endif
#ifdef B1500000
            case 1500000:
                speed = B1500000;
                break;
#endif
#ifdef B2000000
            case 2000000:
                speed = B2000000;
                break;
#endif
#ifdef B2500000
            case 2500000:
                speed = B2500000;
                break;
#endif
#ifdef B3000000
            case 3000000:
                speed = B3000000;
                break;
#endif
#ifdef B3500000
            case 3500000:
                speed = B3500000;
                break;
#endif
#ifdef B4000000
            case 4000000:
                speed = B4000000;
                break;
#endif
            default:
                speed = baudrate;
                break;
        }

        cfsetispeed(&tty, speed);
        cfsetospeed(&tty, speed);
        tcsetattr(serial_port, TCSANOW, &tty);

    }

    int LinuxSerial::setPath(std::string serialPath) {
        if (serialPath != "")
            path = serialPath;
        return 0;
    }

    void LinuxSerial::end() {
        if (serial_port != -1)
            close(serial_port);
    }

    int LinuxSerial::available(void) {
        int bytes;
        int ret = ioctl(serial_port, FIONREAD, &bytes);
        if (ret == -1) {
            // ioctl failed, likely due to calling available on an invalid file descriptor (EBADF)
            return 0;
        }
        return bytes;
    }

    int LinuxSerial::peek(void) {
        if (hasPeeked) return peekedByte;
        unsigned char c = 0;
        ssize_t rv = ::read(serial_port, &c, 1);
        if (rv == 1) {
            hasPeeked = true;
            peekedByte = (int)c;
            return peekedByte;
        }
        return -1;
    }

    int LinuxSerial::read(void) {
        if (hasPeeked) {
            hasPeeked = false;
            return peekedByte;
        }
        unsigned char c = 0;
        ssize_t rv = ::read(serial_port, &c, 1);
        if (rv == 1) return (int)c;
        return -1;
    }

    void LinuxSerial::flush(void) {
    }

    size_t LinuxSerial::write(uint8_t c) {
        ::write(serial_port, &c, 1);
        return 1;
    }

    LinuxSerial::operator bool() {
        // Returns true if the port is ready for use
        return serial_port != -1;
    }



    //simulated serial for log output:
    void SimSerial::begin(unsigned long baudrate, uint16_t config) {
        // Ignore baudrate and config on linux (for now)
        // FIXME open file descriptor
    }

    void SimSerial::end() {
        // FIXME - close file descriptor
    }

    int SimSerial::available(void) {
        return 0;
    }

    int SimSerial::peek(void) {
        return -1;
    }

    int SimSerial::read(void) {
        return -1;
    }

    void SimSerial::flush(void) {
    }

    size_t SimSerial::write(uint8_t c) {
        putchar(c);
        return 1;
    }

    SimSerial::operator bool() {
        // Returns true if the port is ready for use
        return true;
    }
}
