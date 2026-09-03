#include "Led.h"

Led::Led(int ledPin)
{
    _ledPin = ledPin;
    pinMode(_ledPin, OUTPUT);
}

void Led::hardwareErrorMessage()
{
    digitalWrite(_ledPin, HIGH);
}

void Led::errorMessage(ErrorCode errorCode)
{
    if (errorCode == HARDWARE_ERROR)
        hardwareErrorMessage();
}
