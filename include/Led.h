# pragma once

#include <Arduino.h>

enum class ErrorCode {
    NONE,
    HARDWARE_ERROR,
    WIFI_ERROR,
    HTTP_ERROR
};

class Led
{
    static constexpr int TOGGLES_HARDWARE = 2;
    static constexpr unsigned long INTERVAL_HARDWARE = 2000;
    static constexpr int TOGGLES_WIFI = 4;
    static constexpr unsigned long INTERVAL_WIFI = 250;
    static constexpr int TOGGLES_HTTP = 6;
    static constexpr unsigned long INTERVAL_HTTP = 250;

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
    void update();
    bool isLedBusy();
};
