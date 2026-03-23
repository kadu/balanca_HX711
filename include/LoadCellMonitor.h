#ifndef LOADCELL_MONITOR_H
#define LOADCELL_MONITOR_H

#include <HX711_ADC.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "DisplayManager.h"
#include "Recipe.h"

class LoadCellMonitor;
extern LoadCellMonitor* globalLoadCellPtr;
void IRAM_ATTR hx711DataReadyISR();

class LoadCellMonitor {
private:
    HX711_ADC scale;
    uint8_t dout, sck, buzzerPin;
    float calibrationFactor;
    DisplayManager& display;
    
    unsigned long lastDisplayUpdate = 0;
    const int displayRefreshInterval = 80; // Um pouco mais rápido para fluidez
    const int eepromAddr = 0;

    unsigned long timerStartTime = 0;
    unsigned long timerElapsedTime = 0;
    bool timerRunning = false;
    const float weightThreshold = 1.5;
    Recipe currentRecipe;

    int lastStepIndex = -1;
    bool recipeFinishedSoundPlayed = false;

public:
    LoadCellMonitor(uint8_t d, uint8_t s, float defaultCf, DisplayManager& dm, uint8_t bPin) 
        : scale(d, s), dout(d), sck(s), buzzerPin(bPin), calibrationFactor(defaultCf), display(dm) {
        globalLoadCellPtr = this;
        currentRecipe.name = "Livre";
    }

    void setRecipe(const Recipe& r) { 
        currentRecipe = r;
        lastStepIndex = -1;
        recipeFinishedSoundPlayed = false;
    }
    
    float getWeight() { return scale.getData(); }
    const char* getRecipe() { return currentRecipe.name.c_str(); }
    unsigned long getElapsedTime() { return timerElapsedTime; }

    void begin() {
        scale.begin();
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW);

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
        
        // --- Cálculo da Etapa Atual (Acumulada) ---
        int targetWeight = 0;
        int targetTime = 0;
        int currentStepIndex = -1;
        bool recipeEnded = false;
        
        if (currentRecipe.steps.size() > 0) {
            int accTime = 0;
            int accWeight = 0;
            bool foundStep = false;
            for (int i = 0; i < (int)currentRecipe.steps.size(); i++) {
                accTime += currentRecipe.steps[i].time;
                accWeight += currentRecipe.steps[i].amount;
                if ((timerElapsedTime / 1000) < (unsigned long)accTime) {
                    targetWeight = accWeight;
                    targetTime = accTime;
                    currentStepIndex = i;
                    foundStep = true;
                    break;
                }
            }
            if (!foundStep) {
                targetWeight = accWeight;
                targetTime = accTime;
                recipeEnded = timerRunning;
            }
        }

        // --- Lógica de Som ---
        if (timerRunning) {
            // Mudança de passo
            if (currentStepIndex != lastStepIndex && currentStepIndex != -1) {
                if (lastStepIndex != -1) { 
                    tone(buzzerPin, 2500, 300); // Frequência maior e mais longa
                }
                lastStepIndex = currentStepIndex;
            }
            // Fim de Receita
            if (recipeEnded && !recipeFinishedSoundPlayed) {
                for(int i=0; i<3; i++) {
                    tone(buzzerPin, 2500, 100);
                    delay(150);
                }
                recipeFinishedSoundPlayed = true;
            }
        }

        // --- LINHA 0-15: ÁREA AMARELA (Cronômetro e Alvos) ---
        oled.setTextSize(1);
        
        // Cronômetro Centralizado no topo
        unsigned long totalSeconds = timerElapsedTime / 1000;
        char timeBuf[12];
        sprintf(timeBuf, "%02u:%02u.%01u", (unsigned int)(totalSeconds / 60), (unsigned int)(totalSeconds % 60), (unsigned int)((timerElapsedTime % 1000) / 100));
        oled.setCursor(35, 0); 
        oled.print(timeBuf);

        // Ícone WiFi Discreto (Canto superior direito)
        if (WiFi.status() == WL_CONNECTED) {
            oled.drawPixel(125, 2, SSD1306_WHITE);
            oled.drawPixel(123, 4, SSD1306_WHITE);
            oled.drawPixel(127, 4, SSD1306_WHITE);
        }

        // Informação do Alvo (Linha 10 Amarela)
        oled.setCursor(0, 9);
        if (currentRecipe.steps.size() > 0) {
            char targetBuf[32];
            if (recipeEnded) {
                sprintf(targetBuf, "FIM! Total: %dml", targetWeight);
            } else {
                sprintf(targetBuf, "ALVO: %dml | %02u:%02u", targetWeight, (unsigned int)(targetTime/60), (unsigned int)(targetTime%60));
            }
            oled.print(targetBuf);
        } else {
            oled.print("MODO BALANCA");
        }

        // --- LINHA 16: GAP (NÃO USAR) ---

        // --- LINHAS 17-63: ÁREA AZUL (Peso e Nome) ---
        
        // Exibição do Peso Grande (Centralizado)
        if (absWeight < 0.5) weight = 0.0;
        oled.setCursor(10, 22); // Começa na área azul
        if (absWeight < 1000.0) {
            oled.setTextSize(4);
            oled.print((int)weight);
            oled.setTextSize(2);
            oled.print("g");
        } else {
            oled.setTextSize(3);
            oled.print((int)weight);
            oled.setTextSize(1);
            oled.print("g");
        }

        // Rodapé: Nome da Receita (Pequeno)
        oled.setTextSize(1);
        oled.setCursor(0, 56);
        oled.print(currentRecipe.name);

        if (scale.getTareStatus()) {
            oled.setCursor(110, 56);
            oled.print("[T]");
        }

        oled.display();
    }

    void tare() {
        scale.tareNoDelay();
        timerRunning = false;
        timerElapsedTime = 0;
        timerStartTime = 0;
        lastStepIndex = -1;
        recipeFinishedSoundPlayed = false;
    }

    void stop() {
        detachInterrupt(digitalPinToInterrupt(dout));
    }
};

#endif
