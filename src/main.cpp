#include <FastLED.h>
#include <Arduino.h>
#include "DisplayManager.h"
#include "LoadCellMonitor.h"
#include "Button.h"
#include "MenuManager.h"

// Configuration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C

#define LOADCELL_DOUT 14 // D5
#define LOADCELL_SCK 12  // D6
#define CALIBRATION_FACTOR 415.0

#define BTN_1_PIN 0 // D3
#define BTN_2_PIN 2 // D4

// Objects
DisplayManager display(OLED_WIDTH, OLED_HEIGHT, OLED_ADDR);
LoadCellMonitor loadCellMonitor(LOADCELL_DOUT, LOADCELL_SCK, CALIBRATION_FACTOR, display);
MenuManager menuManager(display);
Button btn1(BTN_1_PIN);
Button btn2(BTN_2_PIN);

// Ponteiro global e ISR para a balança
LoadCellMonitor* globalLoadCellPtr = nullptr;
void IRAM_ATTR hx711DataReadyISR() {
    if (globalLoadCellPtr) {
        globalLoadCellPtr->handleInterrupt();
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    if (!display.begin()) {
        Serial.println(F("OLED failed"));
    }

    btn1.begin();
    btn2.begin();
    
    display.showStatus("INICIALIZANDO", "Aguarde...", "Balança");
    loadCellMonitor.begin();
    loadCellMonitor.setRecipe(menuManager.getSelectedName());
    
    display.showStatus("PRONTO", "Modo Balança", "D3 p/ Menu");
    delay(1000);
}

void loop() {
    // A balança precisa atualizar sempre para não perder amostras
    loadCellMonitor.updateScale();

    if (menuManager.isActive()) {
        // --- MODO MENU ---
        menuManager.draw();

        if (btn1.wasClicked()) {
            menuManager.nextOption();
        }
        if (btn2.wasClicked()) {
            menuManager.selectCurrent();
            // Volta para a balança com a nova receita e zera tudo
            loadCellMonitor.setRecipe(menuManager.getSelectedName());
            loadCellMonitor.tare();
        }
    } else {
        // --- MODO BALANÇA ---
        loadCellMonitor.draw();

        // Botão 1: Entra no Menu
        if (btn1.wasClicked()) {
            menuManager.activate();
        }
        // Botão 2: Tara + Reset Tempo
        if (btn2.wasClicked()) {
            loadCellMonitor.tare();
        }
    }

    // Estabilidade do sistema
    yield(); 
}
