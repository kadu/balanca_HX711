#include "NetworkManager.h"
#include <WiFiManager.h>

NetworkManager::NetworkManager(DisplayManager& dm) : mqttClient(espClient), display(dm) {}

void NetworkManager::begin() {
    loadSettings();
    if (mqtt_server[0] != '\0' && mqtt_server[0] != (char)255) {
        mqttClient.setServer(mqtt_server, 1883);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin();
}

void NetworkManager::loadSettings() {
    EEPROM.begin(512);
    EEPROM.get(eeprom_mqtt_start, mqtt_server);
    EEPROM.get(eeprom_mqtt_start + 40, mqtt_enabled);
    if (mqtt_enabled[0] != '0' && mqtt_enabled[0] != '1') {
        mqtt_enabled[0] = '0';
        mqtt_enabled[1] = '\0';
    }
}

void NetworkManager::saveSettings() {
    EEPROM.put(eeprom_mqtt_start, mqtt_server);
    EEPROM.put(eeprom_mqtt_start + 40, mqtt_enabled);
    EEPROM.commit();
}

void NetworkManager::setupConfigPortal() {
    WiFiManager wm;
    
    // Habilita debug detalhado no Serial
    wm.setDebugOutput(true);
    
    // Callback para quando um cliente se conecta ao AP (SoftAP)
    static WiFiEventHandler stationConnectedHandler = WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected& evt) {
        Serial.printf("DEBUG: Cliente Conectado! MAC: %02X:%02u:%02X:%02X:%02X:%02X\n", 
            evt.mac[0], evt.mac[1], evt.mac[2], evt.mac[3], evt.mac[4], evt.mac[5]);
    });

    // Configura o portal
    wm.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    
    display.showStatus("MODO CONFIG", "Conecte: BalancaAP", "IP: 192.168.4.1");
    Serial.println("Aguardando conexao no IP 192.168.4.1...");
    
    WiFiManagerParameter custom_mqtt_server("server", "Servidor MQTT", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_en("mqtt_en", "MQTT (1=Lig / 0=Desl)", mqtt_enabled, 2);

    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_en);

    // Tenta abrir o portal (bloqueante)
    if (!wm.startConfigPortal("BalancaAP")) {
        Serial.println("Falha no portal ou timeout");
        delay(1000);
        ESP.restart();
    }
    
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_enabled, custom_mqtt_en.getValue());
    saveSettings();
    
    display.showStatus("SUCESSO", "WiFi Configurado", "Reiniciando...");
    delay(2000);
    ESP.restart();
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkManager::loop() {
    if (isConnected() && mqtt_enabled[0] == '1') {
        if (!mqttClient.connected()) {
            static unsigned long lastMqttRetry = 0;
            if (millis() - lastMqttRetry > 15000) {
                lastMqttRetry = millis();
                mqttClient.connect("BalancaIoT");
            }
        }
        mqttClient.loop();
    }
}

void NetworkManager::publishWeight(float weight) {
    if (mqttClient.connected()) {
        char buf[10];
        dtostrf(weight, 4, 1, buf);
        mqttClient.publish("balanca/peso", buf);
    }
}
