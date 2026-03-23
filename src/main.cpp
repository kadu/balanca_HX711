#include <FastLED.h>
#include <Arduino.h>

#include "DisplayManager.h"
#include "LoadCellMonitor.h"
#include "WebServerManager.h"
#include "NetworkManager.h"
#include "Button.h"
#include "MenuManager.h"
#include "OTAManager.h"
#include "RecipeManager.h"

// Configuration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C

#define LOADCELL_DOUT 14
#define LOADCELL_SCK 12
#define CALIBRATION_FACTOR 415.0
#define BUZZER_PIN 13 // D7

#define BTN_1_PIN 0
#define BTN_2_PIN 2

// Objects
DisplayManager display(OLED_WIDTH, OLED_HEIGHT, OLED_ADDR);
LoadCellMonitor loadCellMonitor(LOADCELL_DOUT, LOADCELL_SCK, CALIBRATION_FACTOR, display, BUZZER_PIN);
RecipeManager recipeManager;
MenuManager menuManager(display, recipeManager);
NetworkManager networkManager(display);
WebServerManager webServer(80, loadCellMonitor, networkManager, recipeManager);
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
    
    recipeManager.begin(); // Inicializa LittleFS e carrega receitas
    networkManager.begin();
    loadCellMonitor.begin();
    
    // Carrega a receita inicial selecionada no menu
    String initialName = menuManager.getSelectedName();
    const auto& setupRecipes = recipeManager.getRecipes();
    bool setupFound = false;
    for (const auto& r : setupRecipes) {
        if (r.name == initialName) {
            loadCellMonitor.setRecipe(r);
            setupFound = true;
            break;
        }
    }
    if (!setupFound) {
        Recipe livre; livre.name = initialName;
        loadCellMonitor.setRecipe(livre);
    }

    webServer.begin();
    otaManager.begin("BalancaIoT");
    
    display.showStatus("SISTEMA OK", "Modo Balanca", "D3(segure): WiFi");
    delay(1000);
}

void loop() {
    loadCellMonitor.updateScale();
    networkManager.loop();
    webServer.handle(); 
    otaManager.handle();

    if (btn1.isHeldFor(10000)) {
        loadCellMonitor.stop();
        webServer.stop();
        networkManager.setupConfigPortal();
    }

    if (menuManager.isActive()) {
        menuManager.draw();
        if (btn1.wasClicked()) menuManager.nextOption();
        if (btn2.wasClicked()) {
            menuManager.selectCurrent();
            String sel = menuManager.getSelectedName();
            
            // Busca a receita completa para passar ao monitor
            const auto& recipes = recipeManager.getRecipes();
            bool found = false;
            for (const auto& r : recipes) {
                if (r.name == sel) {
                    loadCellMonitor.setRecipe(r);
                    found = true;
                    break;
                }
            }
            if (!found) {
                Recipe livre; livre.name = "Livre";
                loadCellMonitor.setRecipe(livre);
            }

            loadCellMonitor.tare();
            networkManager.publishRecipe(sel.c_str());
        }
    } else {
        loadCellMonitor.draw();
        
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
