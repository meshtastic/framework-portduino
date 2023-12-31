//
// Created by kevinh on 9/1/20.
//

#ifndef PORTDUINO_LINUXSERIAL_H
#define PORTDUINO_LINUXSERIAL_H

#include "HardwareSerial.h"
#include <string>

namespace arduino {
    class LinuxSerial : public HardwareSerial {
        int serial_port = -1;
        std::string path;
    public:
        virtual void begin(unsigned long baudrate) { begin(baudrate, SERIAL_8N1 ); }
        virtual void begin(unsigned long baudrate, uint16_t config);
        virtual int setPath(std::string serialPath);
        virtual void end();
        virtual int available(void);
        virtual int peek(void);
        virtual int read(void);
        virtual void flush(void);
        virtual size_t write(uint8_t);
        using Print::write; // pull in write(str) and write(buf, size) from Print
        virtual operator bool();
    };

        class SimSerial : public HardwareSerial {
    public:
        virtual void begin(unsigned long baudrate) { begin(baudrate, SERIAL_8N1 ); }
        virtual void begin(unsigned long baudrate, uint16_t config);
        virtual void end();
        virtual int available(void);
        virtual int peek(void);
        virtual int read(void);
        virtual void flush(void);
        virtual size_t write(uint8_t);
        using Print::write; // pull in write(str) and write(buf, size) from Print
        virtual operator bool();
    };

    extern LinuxSerial Serial1;
    extern SimSerial Serial;
}

#endif //PORTDUINO_LINUXSERIAL_H
