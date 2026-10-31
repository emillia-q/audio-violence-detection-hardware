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
    if (errorCode == ErrorCode::NONE)
        return;

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
        maxToggles = TOGGLES_HARDWARE;
        interval = INTERVAL_HARDWARE;
    } else if (errorCode == ErrorCode::WIFI_ERROR) {
        maxToggles = TOGGLES_WIFI;
        interval = INTERVAL_WIFI;
    } else if (errorCode == ErrorCode::HTTP_ERROR) {
        maxToggles = TOGGLES_HTTP;
        interval = INTERVAL_HTTP;
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

bool Led::isLedBusy()
{
    return currentError != ErrorCode::NONE;
}
