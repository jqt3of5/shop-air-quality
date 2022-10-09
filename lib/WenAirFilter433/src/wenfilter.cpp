//
// Created by jqt3o on 10/9/2022.
//

#include "wenfilter.h"
#include <Arduino.h>

WenFilter::WenFilter(int pin)
{
    _pin = pin;
}

void WenFilter::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void WenFilter::writePreamble() const {
    for (int i = 0; i < 25; ++i)
    {
        digitalWrite(_pin, HIGH);
        delayMicroseconds(300 + 30);

        digitalWrite(_pin, LOW);
        delayMicroseconds(340 - 30);
    }
}

void inline WenFilter::writeBlock(int16_t bits) const
{
    for (int i = sizeof(bits)*8-1; i >= 0; i--)
    {
        bool v = bits & (1 << i);

        digitalWrite(_pin, HIGH);
        delayMicroseconds(v ? 970 : 370);
        digitalWrite(_pin, LOW);
        delayMicroseconds(v ? 310: 910);
    }
}

void WenFilter::writeCommand(WenCommand command) {

    writePreamble();
    delay(10);
    //The remote will repeat the command three times
    writeBlock(command);
    delay(10);
    writeBlock(command);
    delay(10);
    writeBlock(command);
}

void WenFilter::setOffClear() {
    writeCommand(WenCommand::Off);
}

void WenFilter::setOnState(WenFilterSpeed speed, WenFilterTime time) {
    writeCommand(WenCommand::Off);

    for (int i = 0; i < speed; ++i) {
        delay(1000);
        writeCommand(WenCommand::Speed);
    }

    for (int i = 0; i < time; ++i) {
        delay(1000);
        writeCommand(WenCommand::Time);
    }
}

