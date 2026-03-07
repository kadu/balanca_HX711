#ifndef LED_TESTER_H
#define LED_TESTER_H

#include <FastLED.h>
#include "DisplayManager.h"

template <uint8_t PIN, uint8_t NUM_LEDS>
class LedTester {
private:
    CRGB leds[NUM_LEDS];
    DisplayManager& display;

    void setAllLeds(CRGB color) {
        fill_solid(leds, NUM_LEDS, color);
        FastLED.show();
    }

public:
    LedTester(DisplayManager& dm) : display(dm) {}

    void begin() {
        FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
        FastLED.setBrightness(50);
        setAllLeds(CRGB::Black);
    }

    void runTest() {
        Serial.println("--- Starting LED Test ---");
        display.showStatus("LED TEST", "Cycling Colors...");

        CRGB colors[] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::White};
        for (CRGB color : colors) {
            setAllLeds(color);
            delay(1000);
        }

        display.showStatus("LED TEST", "Individual LEDs...");
        setAllLeds(CRGB::Black);
        for(int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB::Purple;
            FastLED.show();
            delay(500);
            leds[i] = CRGB::Black;
            FastLED.show();
        }

        display.showStatus("LED TEST", "Rainbow Animation!");
        for(int j = 0; j < 255; j++) {
            fill_rainbow(leds, NUM_LEDS, j, 255/NUM_LEDS);
            FastLED.show();
            delay(20);
        }
        setAllLeds(CRGB::Black);
        Serial.println("--- LED Test Finished ---");
    }
};

#endif
