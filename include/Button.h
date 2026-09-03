# pragma once

enum ButtonEvent {
    NONE,
    SHORT_CLICK,
    LONG_HOLD
};

class Button
{
private:
    int _buttonPin;
public:
    Button(int buttonPin);
};
