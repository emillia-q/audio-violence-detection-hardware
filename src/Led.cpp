#include "Led.h"

Led::Led(int ledPin)
{
    _ledPin = ledPin;
    pinMode(_ledPin, OUTPUT);
}