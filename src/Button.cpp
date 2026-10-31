#include "Button.h"

Button::Button(int buttonPin)
{
    _buttonPin = buttonPin;
    pinMode(_buttonPin, INPUT);
    pressTime = 0;
    isPressed = false;
    longPressHandeled = false;
}

ButtonEvent Button::getEvent()
{
    // Get current state
    bool currentState = (digitalRead(_buttonPin) == LOW);

    // Button press
    if (currentState && !isPressed) {
        isPressed = true;
        pressTime = millis();
        longPressHandeled = false;
    }

    // While holding button
    if (isPressed && currentState) {
        // Long hold
        if (!longPressHandeled && (millis() - pressTime >= LONG_PRESS_DELAY_MS)) {
            longPressHandeled = true;
            return ButtonEvent::LONG_HOLD;
        }
    }

    // Button release
    if (!currentState && isPressed) {
        isPressed = false;
        
        unsigned long pressDuration = millis() - pressTime;
        
        // Debouncing
        if (!longPressHandeled && pressDuration >= DEBOUNCE_DELAY_MS) {
            return ButtonEvent::SHORT_CLICK;
        }
    }

    return ButtonEvent::NONE;
}
