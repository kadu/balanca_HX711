#ifndef LOADCELL_MONITOR_H
#define LOADCELL_MONITOR_H

#include <HX711_ADC.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "DisplayManager.h"

class LoadCellMonitor;
extern LoadCellMonitor* globalLoadCellPtr;
void IRAM_ATTR hx711DataReadyISR();

class LoadCellMonitor {
private:
    HX711_ADC scale;
    uint8_t dout, sck;
    float calibrationFactor;
    DisplayManager& display;
    
    unsigned long lastDisplayUpdate = 0;
    const int displayRefreshInterval = 100;
    const int eepromAddr = 0;

    unsigned long timerStartTime = 0;
    unsigned long timerElapsedTime = 0;
    bool timerRunning = false;
    const float weightThreshold = 1.5;
    char activeRecipe[20] = "Padrao"; // Buffer fixo para segurança

public:
    LoadCellMonitor(uint8_t d, uint8_t s, float defaultCf, DisplayManager& dm) 
        : scale(d, s), dout(d), sck(s), calibrationFactor(defaultCf), display(dm) {
        globalLoadCellPtr = this;
    }

    void setRecipe(const char* name) { 
        strncpy(activeRecipe, name, sizeof(activeRecipe)-1);
        activeRecipe[sizeof(activeRecipe)-1] = '\0';
    }
    
    float getWeight() { return scale.getData(); }
    const char* getRecipe() { return activeRecipe; }
    unsigned long getElapsedTime() { return timerElapsedTime; }

    void begin() {
        scale.begin();
        EEPROM.begin(512);
        float savedCf;
        EEPROM.get(eepromAddr, savedCf);
        if (!isnan(savedCf) && savedCf != 0.0) calibrationFactor = savedCf;
        
        scale.start(2000, true);
        scale.setCalFactor(calibrationFactor);
        attachInterrupt(digitalPinToInterrupt(dout), hx711DataReadyISR, FALLING);
    }

    void handleInterrupt() { scale.update(); }
    void updateScale() { scale.update(); }

    void draw() {
        if (millis() - lastDisplayUpdate < displayRefreshInterval) return;
        lastDisplayUpdate = millis();

        float weight = scale.getData();
        float absWeight = weight > 0 ? weight : -weight;

        if (!timerRunning && absWeight > weightThreshold) {
            timerStartTime = millis();
            timerRunning = true;
        }
        if (timerRunning) {
            timerElapsedTime = millis() - timerStartTime;
        }

        Adafruit_SSD1306& oled = display.getRawDisplay();
        oled.clearDisplay();
        oled.setTextColor(SSD1306_WHITE);
        
        // --- Topo: Cronômetro ---
        oled.setTextSize(1);
        oled.setCursor(0, 0);
        oled.print("TIME: ");
        unsigned long totalSeconds = timerElapsedTime / 1000;
        char timeBuf[16];
        sprintf(timeBuf, "%02u:%02u.%02u", (unsigned int)(totalSeconds / 60), (unsigned int)(totalSeconds % 60), (unsigned int)((timerElapsedTime % 1000) / 10));
        oled.print(timeBuf);
        
        if (WiFi.status() == WL_CONNECTED) {
            oled.drawCircle(120, 5, 2, SSD1306_WHITE);
            oled.drawCircle(120, 5, 4, SSD1306_WHITE);
        }

        // --- Linha 2: Receita ---
        oled.setCursor(0, 12);
        oled.print("REC: ");
        oled.print(activeRecipe);
        
        oled.drawFastHLine(0, 23, 128, SSD1306_WHITE);
        
        // --- Centro: Peso ---
        if (absWeight < 0.5) weight = 0.0;
        oled.setCursor(0, 32);
        if (absWeight < 1000.0) {
            oled.setTextSize(4);
            oled.print((int)weight);
            oled.setTextSize(1);
            oled.print(" g");
        } else {
            oled.setTextSize(3);
            oled.print((int)weight);
            oled.print(" g");
        }

        if (scale.getTareStatus()) {
            oled.setTextSize(1);
            oled.setCursor(105, 55);
            oled.print("[T]");
        }
        oled.display();
    }

    void tare() {
        scale.tareNoDelay();
        timerRunning = false;
        timerElapsedTime = 0;
        timerStartTime = 0;
    }

    void stop() {
        detachInterrupt(digitalPinToInterrupt(dout));
    }
};

#endif
