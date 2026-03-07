#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Wire.h>
#include <Adafruit_GFX.h>

// Handle macro conflict: Adafruit SSD1306 defines BLACK, which conflicts with FastLED's internal types
#ifdef BLACK
#undef BLACK
#endif
#include <Adafruit_SSD1306.h>

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    uint8_t address;

public:
    DisplayManager(uint8_t width, uint8_t height, uint8_t addr) 
        : display(width, height, &Wire, -1), address(addr) {}

    bool begin() {
        if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
            return false;
        }
        display.clearDisplay();
        display.display();
        return true;
    }

    void showStatus(const char* title, const char* msg1 = "", const char* msg2 = "") {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("--- STATUS ---");
        display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
        
        display.setCursor(0, 15);
        display.println(title);
        
        display.setCursor(0, 35);
        display.println(msg1);
        
        display.setCursor(0, 50);
        display.println(msg2);
        
        display.display();
    }

    void clear() {
        display.clearDisplay();
        display.display();
    }

    Adafruit_SSD1306& getRawDisplay() { return display; }
};

#endif
