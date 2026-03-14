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
    char mqtt_enabled[2] = "0"; 
    const int eeprom_mqtt_start = 100;

public:
    NetworkManager(DisplayManager& dm);
    void begin();
    void loadSettings();
    void saveSettings();
    void setupConfigPortal();
    bool isConnected();
    void loop();
    void publishWeight(float weight);
};

#endif
