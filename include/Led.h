# pragma once

#include <Arduino.h>

enum ErrorCode {
    HARDWARE_ERROR,
    WIFI_ERROR,
    HTTP_ERROR
};

class Led
{
    int _ledPin;

public:
    Led(int ledPin);

    void errorMessage(ErrorCode errorCode);
};
