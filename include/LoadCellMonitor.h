#ifndef LOADCELL_MONITOR_H
#define LOADCELL_MONITOR_H

#include <HX711_ADC.h>
#include <EEPROM.h>
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
    const int displayRefreshInterval = 100; // Aumentada a frequência para o cronômetro ser fluido
    const int eepromAddr = 0;

    // Variáveis do Cronômetro
    unsigned long timerStartTime = 0;
    unsigned long timerElapsedTime = 0;
    bool timerRunning = false;
    const float weightThreshold = 1.5; // Gramas para disparar o cronômetro

public:
    LoadCellMonitor(uint8_t d, uint8_t s, float defaultCf, DisplayManager& dm) 
        : scale(d, s), dout(d), sck(s), calibrationFactor(defaultCf), display(dm) {
        globalLoadCellPtr = this;
    }

    void begin() {
        scale.begin();
        EEPROM.begin(512);
        
        float savedCalibrationFactor;
        EEPROM.get(eepromAddr, savedCalibrationFactor);
        
        if (!isnan(savedCalibrationFactor) && savedCalibrationFactor != 0.0) {
            calibrationFactor = savedCalibrationFactor;
        }

        unsigned long stabilizingTime = 2000;
        scale.start(stabilizingTime, true);
        
        if (!scale.getTareTimeoutFlag()) {
            scale.setCalFactor(calibrationFactor);
            attachInterrupt(digitalPinToInterrupt(dout), hx711DataReadyISR, FALLING);
        }
    }

    void handleInterrupt() {
        scale.update();
    }

    void update() {
        scale.update();

        if (millis() > lastDisplayUpdate + displayRefreshInterval) {
            float weight = scale.getData();
            float absWeight = weight > 0 ? weight : -weight;

            // Lógica do Cronômetro
            if (!timerRunning && absWeight > weightThreshold) {
                // Inicia o cronômetro
                timerStartTime = millis();
                timerRunning = true;
            }

            if (timerRunning) {
                timerElapsedTime = millis() - timerStartTime;
            }

            Adafruit_SSD1306& oled = display.getRawDisplay();
            oled.clearDisplay();
            oled.setTextColor(SSD1306_WHITE);
            
            // --- Área Superior (Cronômetro) ---
            oled.setTextSize(1);
            oled.setCursor(0, 6); // Centraliza verticalmente na primeira área
            oled.print("TIME: ");
            
            // Formata o tempo: MM:SS.cc
            unsigned long totalSeconds = timerElapsedTime / 1000;
            unsigned int minutes = totalSeconds / 60;
            unsigned int seconds = totalSeconds % 60;
            unsigned int centiseconds = (timerElapsedTime % 1000) / 10;

            char timeBuf[16];
            sprintf(timeBuf, "%02u:%02u.%02u", minutes, seconds, centiseconds);
            oled.print(timeBuf);
            
            if (scale.getTareStatus()) {
                oled.setCursor(100, 0);
                oled.print("[T]");
            }

            oled.drawFastHLine(0, 23, 128, SSD1306_WHITE);
            
            // --- Área Inferior (Gramas) ---
            oled.setCursor(0, 32);
            if (absWeight < 0.5) weight = 0.0;

            if (weight < 1000.0 && weight > -1000.0) {
                oled.setTextSize(3);
                oled.print((int)weight);
                oled.setTextSize(1);
                oled.print(" g");
            } else {
                oled.setTextSize(2);
                oled.print(weight, 0);
                oled.print(" g");
            }
            
            oled.display();
            lastDisplayUpdate = millis();
        }
    }

    void tare() {
        scale.tareNoDelay();
        // Reseta o cronômetro ao aplicar tara
        timerRunning = false;
        timerElapsedTime = 0;
        timerStartTime = 0;
        Serial.println("Scale Tared and Timer Reset.");
    }
};

#endif
