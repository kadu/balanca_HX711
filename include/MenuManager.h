#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <Arduino.h>
#include "DisplayManager.h"
#include "RecipeManager.h"

class MenuManager {
private:
    int currentIndex = 0;
    int selectedIndex = 0;
    bool active = false;
    DisplayManager& display;
    RecipeManager& recipeManager;
    unsigned long lastDraw = 0;

public:
    MenuManager(DisplayManager& dm, RecipeManager& rm) : display(dm), recipeManager(rm) {}

    void activate() {
        active = true;
        currentIndex = selectedIndex;
    }

    void deactivate() { active = false; }
    bool isActive() const { return active; }

    void nextOption() {
        int count = recipeManager.getRecipes().size();
        if (count == 0) return;
        currentIndex = (currentIndex + 1) % count;
    }

    void selectCurrent() {
        selectedIndex = currentIndex;
        active = false;
    }

    String getSelectedName() const {
        const auto& recipes = recipeManager.getRecipes();
        if (selectedIndex >= 0 && (size_t)selectedIndex < recipes.size()) {
            return recipes[selectedIndex].name;
        }
        return "Padrao";
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

        const auto& recipes = recipeManager.getRecipes();
        if (recipes.empty()) {
            oled.setCursor(5, 30);
            oled.print("Sem receitas...");
        } else {
            // Mostra ate 3 receitas centradas no indice atual
            int start = max(0, currentIndex - 1);
            int end = min((int)recipes.size(), start + 3);
            if (end - start < 3 && recipes.size() > 3) start = max(0, end - 3);

            for (int i = start; i < end; i++) {
                oled.setCursor(5, 20 + ((i - start) * 12));
                if (i == currentIndex) {
                    oled.print("> ");
                } else {
                    oled.print("  ");
                }
                oled.print(recipes[i].name);
            }
        }
        
        oled.setCursor(0, 55);
        oled.print("D3: Prox | D4: Sel");
        oled.display();
    }
};

#endif
