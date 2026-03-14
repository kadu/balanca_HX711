#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
private:
    uint8_t pin;
    bool lastState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;
    unsigned long pressStartTime;
    bool isPressing;

public:
    Button(uint8_t p, unsigned long delay = 50) 
        : pin(p), lastState(HIGH), lastDebounceTime(0), debounceDelay(delay), pressStartTime(0), isPressing(false) {}

    void begin() {
        pinMode(pin, INPUT_PULLUP);
    }

    bool wasClicked() {
        int reading = digitalRead(pin);
        if (reading == LOW && lastState == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
            lastState = LOW;
            lastDebounceTime = millis();
            return true;
        }
        if (reading == HIGH && lastState == LOW && (millis() - lastDebounceTime > debounceDelay)) {
            lastState = HIGH;
            lastDebounceTime = millis();
        }
        return false;
    }

    bool isHeldFor(unsigned long duration) {
        int reading = digitalRead(pin);
        if (reading == LOW) {
            if (!isPressing) {
                pressStartTime = millis();
                isPressing = true;
            }
            if (millis() - pressStartTime >= duration) {
                isPressing = false; // Reset para não disparar múltiplas vezes
                return true;
            }
        } else {
            isPressing = false;
        }
        return false;
    }
};

#endif
