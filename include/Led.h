# pragma once

#include <Arduino.h>

enum ErrorCode {
    NONE,
    HARDWARE_ERROR,
    WIFI_ERROR,
    HTTP_ERROR
};

class Led
{
    int _ledPin;
    ErrorCode currentError;
    unsigned long interval;
    unsigned long previousMillis;
    bool ledState;
    int toggleCount;
    int maxToggles;

public:
    Led(int ledPin);

    void errorMessage(ErrorCode errorCode);
};
