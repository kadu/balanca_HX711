#ifndef BUTTON_TESTER_H
#define BUTTON_TESTER_H

#include <Arduino.h>
#include "DisplayManager.h"

class ButtonTester {
private:
    uint8_t pin1, pin2;
    DisplayManager& display;
    long lastDebounceTime1 = 0, lastDebounceTime2 = 0;
    long debounceDelay = 50;
    int buttonState1 = HIGH, lastButtonState1 = HIGH;
    int buttonState2 = HIGH, lastButtonState2 = HIGH;

public:
    ButtonTester(uint8_t p1, uint8_t p2, DisplayManager& dm) 
        : pin1(p1), pin2(p2), display(dm) {}

    void begin() {
        pinMode(pin1, INPUT_PULLUP);
        pinMode(pin2, INPUT_PULLUP);
    }

    void runTest() {
        Serial.println("--- Starting Button Test ---");
        display.showStatus("BUTTON TEST", "Press D3 or D4", "Waiting...");

        unsigned long startTime = millis();
        while (millis() - startTime < 10000) {
            checkButton(pin1, buttonState1, lastButtonState1, lastDebounceTime1, "BTN 1 (D3)");
            checkButton(pin2, buttonState2, lastButtonState2, lastDebounceTime2, "BTN 2 (D4)");
            delay(10);
        }
        Serial.println("--- Button Test Finished ---");
    }

private:
    void checkButton(uint8_t pin, int& state, int& lastState, long& lastDebounce, const char* name) {
        int reading = digitalRead(pin);
        if (reading != lastState) lastDebounce = millis();
        
        if ((millis() - (unsigned long)lastDebounce) > (unsigned long)debounceDelay) {
            if (reading != state) {
                state = reading;
                if (state == LOW) {
                    char buf[32];
                    sprintf(buf, "%s PRESSED!", name);
                    display.showStatus("BUTTON TEST", buf, "Active...");
                } else {
                    display.showStatus("BUTTON TEST", "Press D3 or D4", "Waiting...");
                }
            }
        }
        lastState = reading;
    }
};

#endif
