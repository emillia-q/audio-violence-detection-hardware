# pragma once

#include <Arduino.h>

enum class ButtonEvent {
    NONE,
    SHORT_CLICK,
    LONG_HOLD
};

class Button
{
private:
    int _buttonPin;
    unsigned long pressTime;
    bool isPressed;
    bool longPressHandeled;

public:
    Button(int buttonPin);
    ButtonEvent getEvent();
};
