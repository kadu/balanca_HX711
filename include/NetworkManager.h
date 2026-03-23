#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "DisplayManager.h"

class NetworkManager {
private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    DisplayManager& display;
    
    char mqtt_server[40] = "";
    char mqtt_user[20] = "";
    char mqtt_pass[20] = "";
    char mqtt_topic[40] = "balanca/iot";
    char mqtt_enabled[2] = "0"; 
    char leds_enabled[2] = "1";
    uint8_t led_brightness = 100; // 0-255
    
    const int eeprom_mqtt_start = 100;

public:
    NetworkManager(DisplayManager& dm);
    void begin();
    void loadSettings();
    void saveSettings();
    void setupConfigPortal();
    void updateSettings(const char* server, const char* user, const char* pass, const char* topic, bool enabled, bool ledEnable, uint8_t brightness);
    bool isConnected();
    void loop();
    
    const char* getServer() { return mqtt_server; }
    const char* getUser() { return mqtt_user; }
    const char* getPass() { return mqtt_pass; }
    const char* getTopic() { return mqtt_topic; }
    bool isMqttEnabled() { return mqtt_enabled[0] == '1'; }
    bool isLedEnabled() { return leds_enabled[0] == '1'; }
    uint8_t getLedBrightness() { return led_brightness; }

    bool testConnection(const char* s, const char* u, const char* p, const char* t);

    void publishStatus(const char* status);
    void publishRecipe(const char* recipe);
    void publishWeight(float weight);
};

#endif
