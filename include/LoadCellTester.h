#ifndef LOADCELL_TESTER_H
#define LOADCELL_TESTER_H

#include <HX711_ADC.h>
#include "DisplayManager.h"

class LoadCellTester {
private:
    HX711_ADC scale;
    uint8_t dout, sck;
    float calibrationFactor;
    DisplayManager& display;

public:
    LoadCellTester(uint8_t d, uint8_t s, float cf, DisplayManager& dm) 
        : scale(d, s), dout(d), sck(s), calibrationFactor(cf), display(dm) {}

    void begin() {
        scale.begin();
    }

    void runTest() {
        Serial.println("--- Starting LoadCell Test ---");
        display.showStatus("LOADCELL TEST", "Checking HX711...", "Wait...");
        
        unsigned long stabilizingTime = 2000; 
        scale.start(stabilizingTime, true);
        
        if (!scale.getTareTimeoutFlag()) {
            scale.setCalFactor(calibrationFactor);
            display.showStatus("LOADCELL TEST", "TARE: Zeroing", "Empty Scale");
            
            unsigned long startTime = millis();
            while (millis() - startTime < 15000) {
                scale.update();
                if (millis() % 250 == 0) { // Atualiza display a cada 250ms
                    float weight = scale.getData();
                    
                    char buf1[20];
                    dtostrf(weight, 6, 1, buf1);
                    strcat(buf1, " g");
                    display.showStatus("LOADCELL TEST", "Weight:", buf1);
                }
                delay(1);
            }
        } else {
            display.showStatus("LOADCELL ERROR", "Timeout!", "Check Wiring");
            delay(3000);
        }
        Serial.println("--- LoadCell Test Finished ---");
    }
};

#endif
