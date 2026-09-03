#include "Button.h"

Button::Button(int buttonPin)
{
    _buttonPin = buttonPin;
    pinMode(_buttonPin, INPUT);
    pressTime = 0;
    isPressed = false;
    longPressHandeled = false;
}