#include "Led.h"

Led::Led(int ledPin)
{
    _ledPin = ledPin;
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
    currentError = NONE;
}

void Led::errorMessage(ErrorCode errorCode)
{
    // Prevents overriding currently displayed error
    if (currentError != NONE) 
        return;

    currentError = errorCode;
    previousMillis = millis();

    // Start with led on
    ledState = HIGH;
    digitalWrite(_ledPin, ledState);
    toggleCount = 1;

    if (errorCode == HARDWARE_ERROR) {
        maxToggles = 2;
        interval = 2000;
    } else if (errorCode == WIFI_ERROR) {
        maxToggles = 4;
        interval = 250;
    } else if (errorCode == HTTP_ERROR) {
        maxToggles = 6;
        interval = 250;
    }
}
