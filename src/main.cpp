#include <FastLED.h>
#include <Arduino.h>
#include "DisplayManager.h"
#include "LoadCellMonitor.h"
#include "Button.h"

// Configuration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C

#define LOADCELL_DOUT 14 // GPIO14 (D5)
#define LOADCELL_SCK 12  // GPIO12 (D6)
#define CALIBRATION_FACTOR 415.0

#define TARE_BUTTON_PIN 2 // GPIO2 (D4)

// Objects
DisplayManager display(OLED_WIDTH, OLED_HEIGHT, OLED_ADDR);
LoadCellMonitor loadCellMonitor(LOADCELL_DOUT, LOADCELL_SCK, CALIBRATION_FACTOR, display);
Button tareButton(TARE_BUTTON_PIN);

// Ponteiro global e ISR para a balança
LoadCellMonitor* globalLoadCellPtr = nullptr;
void IRAM_ATTR hx711DataReadyISR() {
    if (globalLoadCellPtr) {
        globalLoadCellPtr->handleInterrupt();
    }
}

void setup() {
    delay(500);
    Serial.begin(115200);
    while (!Serial && millis() < 5000);

    Serial.println("\n--- LoadCell Monitor with Tare Button ---");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // Off

    if (!display.begin()) {
        Serial.println(F("SSD1306 allocation failed"));
    }

    tareButton.begin();
    loadCellMonitor.begin();

    display.showStatus("SYSTEM READY", "Press D4 to TARE", "Monitoring...");
    delay(1500);
}

void loop() {
    // Atualiza a balança (Processa ISR e cálculos)
    loadCellMonitor.update();

    // Verifica se o botão de Tara foi clicado
    if (tareButton.wasClicked()) {
        Serial.println("Tare command received from Button D4");
        loadCellMonitor.tare();
    }

    // Pisca o LED interno ocasionalmente para indicar atividade
    if (millis() % 2000 < 100) {
        digitalWrite(LED_BUILTIN, LOW); // On
    } else {
        digitalWrite(LED_BUILTIN, HIGH); // Off
    }
}
