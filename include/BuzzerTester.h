#ifndef BUZZER_TESTER_H
#define BUZZER_TESTER_H

#include <Arduino.h>
#include "DisplayManager.h"

class BuzzerTester {
private:
    uint8_t pin;
    DisplayManager& display;

public:
    BuzzerTester(uint8_t p, DisplayManager& dm) : pin(p), display(dm) {}

    void begin() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    void runTest() {
        Serial.println("--- Starting Buzzer Test ---");
        display.showStatus("BUZZER TEST", "Beeping...", "High Tone");
        
        for(int i = 0; i < 3; i++) {
            digitalWrite(pin, HIGH);
            delay(100);
            digitalWrite(pin, LOW);
            delay(100);
        }
        
        display.showStatus("BUZZER TEST", "Tone Sweep", "Siren Effect");
        for(int freq = 500; freq < 2000; freq += 100) {
            tone(pin, freq);
            delay(50);
        }
        for(int freq = 2000; freq > 500; freq -= 100) {
            tone(pin, freq);
            delay(50);
        }
        noTone(pin);
        Serial.println("--- Buzzer Test Finished ---");
    }
};

#endif
