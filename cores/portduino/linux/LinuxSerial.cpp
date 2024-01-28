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

        cfsetispeed(&tty, baudrate);
        cfsetospeed(&tty, baudrate);
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
        return -1;
    }

    int LinuxSerial::read(void) {
        int buf;
        ::read(serial_port, &buf, 1);
        return buf;
    }

    void LinuxSerial::flush(void) {
    }

    size_t LinuxSerial::write(uint8_t c) {
        ::write(serial_port, &c, 1);
        return 1;
    }

    LinuxSerial::operator bool() {
        // Returns true if the port is ready for use
        return true;
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
