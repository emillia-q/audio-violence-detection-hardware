#include "Led.h"

Led::Led(int ledPin)
{
    _ledPin = ledPin;
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW);
    currentError = ErrorCode::NONE;
}

void Led::errorMessage(ErrorCode errorCode)
{
    // Prevents overriding currently displayed error
    if (currentError != ErrorCode::NONE) 
        return;

    currentError = errorCode;
    previousMillis = millis();

    // Start with led on
    ledState = HIGH;
    digitalWrite(_ledPin, ledState);
    toggleCount = 1;

    if (errorCode == ErrorCode::HARDWARE_ERROR) {
        maxToggles = 2;
        interval = 2000;
    } else if (errorCode == ErrorCode::WIFI_ERROR) {
        maxToggles = 4;
        interval = 250;
    } else if (errorCode == ErrorCode::HTTP_ERROR) {
        maxToggles = 6;
        interval = 250;
    }
}

void Led::update()
{
    if (currentError == ErrorCode::NONE)
        return;

    if (millis() - previousMillis >= interval) {
        previousMillis = millis();

        if (toggleCount < maxToggles) {
            ledState = !ledState; // Toggle state
            digitalWrite(_ledPin, ledState);
            toggleCount++;
        } else {
            // Reached the limit
            digitalWrite(_ledPin, LOW);
            currentError = ErrorCode::NONE;
        }
    }
}
