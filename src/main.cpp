#include <FastLED.h>
#include <Arduino.h>

#include "DisplayManager.h"
#include "LoadCellMonitor.h"
#include "WebServerManager.h"
#include "NetworkManager.h"
#include "Button.h"
#include "MenuManager.h"
#include "OTAManager.h"

// Configuration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C

#define LOADCELL_DOUT 14
#define LOADCELL_SCK 12
#define CALIBRATION_FACTOR 415.0

#define BTN_1_PIN 0
#define BTN_2_PIN 2

// Objects
DisplayManager display(OLED_WIDTH, OLED_HEIGHT, OLED_ADDR);
LoadCellMonitor loadCellMonitor(LOADCELL_DOUT, LOADCELL_SCK, CALIBRATION_FACTOR, display);
MenuManager menuManager(display);
NetworkManager networkManager(display);
WebServerManager webServer(80, loadCellMonitor, networkManager);
OTAManager otaManager(display, loadCellMonitor);
Button btn1(BTN_1_PIN);
Button btn2(BTN_2_PIN);

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
    
    networkManager.begin();
    loadCellMonitor.begin();
    loadCellMonitor.setRecipe(menuManager.getSelectedName());
    webServer.begin();
    otaManager.begin("BalancaIoT"); // Nome que aparecerá na rede
    
    display.showStatus("SISTEMA OK", "Modo Balanca", "D3(segure): WiFi");
    delay(1000);
}

void loop() {
    // Processamento essencial
    loadCellMonitor.updateScale();
    networkManager.loop();
    webServer.handle(); 
    otaManager.handle(); // Verifica se há atualização disponível

    // Detecção de pressionamento longo (10 segundos) no Botão 1
    if (btn1.isHeldFor(10000)) {
        loadCellMonitor.stop(); // Para interrupções para segurança do rádio
        webServer.stop();       // Libera a porta 80 para o WiFiManager
        networkManager.setupConfigPortal();
    }

    if (menuManager.isActive()) {
        menuManager.draw();
        if (btn1.wasClicked()) menuManager.nextOption();
        if (btn2.wasClicked()) {
            menuManager.selectCurrent();
            loadCellMonitor.setRecipe(menuManager.getSelectedName());
            loadCellMonitor.tare();
            networkManager.publishRecipe(menuManager.getSelectedName());
        }
    } else {
        loadCellMonitor.draw();
        
        // Publica peso no MQTT apenas quando o valor inteiro muda (reduz tráfego)
        static int lastPublishedWeight = -999;
        int currentWeightInt = (int)round(loadCellMonitor.getWeight());
        if (currentWeightInt != lastPublishedWeight) {
            networkManager.publishWeight((float)currentWeightInt);
            lastPublishedWeight = currentWeightInt;
        }

        if (btn1.wasClicked()) menuManager.activate();
        if (btn2.wasClicked()) loadCellMonitor.tare();
    }

    yield();
}
