#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
private:
    uint8_t pin;
    bool lastState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

public:
    Button(uint8_t p, unsigned long delay = 50) 
        : pin(p), lastState(HIGH), lastDebounceTime(0), debounceDelay(delay) {}

    void begin() {
        pinMode(pin, INPUT_PULLUP);
    }

    bool isPressed() {
        bool reading = digitalRead(pin);
        bool pressed = false;

        if (reading != lastState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (reading == LOW && lastState == HIGH) {
                pressed = true;
            }
            lastState = reading;
        }
        
        // Atualiza o estado sem disparar novamente até soltar e apertar
        if ((millis() - lastDebounceTime) > debounceDelay) {
            lastState = reading;
        }

        return pressed;
    }
    
    // Versão simplificada para detecção de clique único
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
};

#endif
