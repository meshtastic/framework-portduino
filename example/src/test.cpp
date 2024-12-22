//
// Created by kevinh on 9/1/20.
//

#include "Arduino.h"

#define TEST_PIN 7

static SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE0);

void setup(void) {
    Serial.begin(115200);

    Serial.println("Starting Portduino test");

    pinMode(TEST_PIN, OUTPUT);
    digitalWrite(TEST_PIN, LOW);

    SPI.begin();

    SPI.beginTransaction(spiSettings);
    uint8_t testBuf[4];
    SPI.transfer(testBuf, sizeof(testBuf));
    SPI.endTransaction();

    Wire.begin();

    Wire.beginTransmission(0x55);
    Wire.write(5);
    Wire.endTransmission();

    Serial.println("Done with setup");
}

void loop(void) {
    digitalWrite(TEST_PIN, digitalRead(TEST_PIN) == LOW ? HIGH : LOW);
}