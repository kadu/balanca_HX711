#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <Arduino.h>
#include "DisplayManager.h"

class MenuManager {
private:
    const char* options[3] = {"Receita 1", "Receita 2", "Receita 3"};
    int currentIndex = 0;
    int selectedIndex = 0;
    bool active = false;
    DisplayManager& display;
    unsigned long lastDraw = 0;

public:
    MenuManager(DisplayManager& dm) : display(dm) {}

    void activate() {
        active = true;
        currentIndex = selectedIndex;
    }

    void deactivate() { active = false; }
    bool isActive() const { return active; }

    void nextOption() {
        currentIndex = (currentIndex + 1) % 3;
    }

    void selectCurrent() {
        selectedIndex = currentIndex;
        active = false;
    }

    const char* getSelectedName() const {
        return options[selectedIndex];
    }

    void draw() {
        if (millis() - lastDraw < 100) return;
        lastDraw = millis();

        Adafruit_SSD1306& oled = display.getRawDisplay();
        oled.clearDisplay();
        oled.setTextColor(SSD1306_WHITE);
        
        oled.setTextSize(1);
        oled.setCursor(0, 0);
        oled.println("--- MENU RECEITAS ---");
        oled.drawFastHLine(0, 10, 128, SSD1306_WHITE);

        for (int i = 0; i < 3; i++) {
            oled.setCursor(5, 20 + (i * 12));
            if (i == currentIndex) {
                oled.print("> ");
            } else {
                oled.print("  ");
            }
            oled.print(options[i]);
        }
        
        oled.setCursor(0, 55);
        oled.print("D3: Prox | D4: Sel");
        oled.display();
    }
};

#endif
