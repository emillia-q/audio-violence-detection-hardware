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

    // Individual error codes
    void hardwareErrorMessage();

public:
    Led(int ledPin);

    void errorMessage(ErrorCode errorCode);
};
