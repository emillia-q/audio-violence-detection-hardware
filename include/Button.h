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
    static constexpr unsigned long DEBOUNCE_DELAY_MS = 50;
    static constexpr unsigned long LONG_PRESS_DELAY_MS = 3000;

    int _buttonPin;
    unsigned long pressTime;
    bool isPressed;
    bool longPressHandeled;

public:
    Button(int buttonPin);
    ButtonEvent getEvent();
};
