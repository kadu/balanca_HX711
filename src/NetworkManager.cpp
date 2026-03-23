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
    EEPROM.get(eeprom_mqtt_start + 40, mqtt_user);
    EEPROM.get(eeprom_mqtt_start + 60, mqtt_pass);
    EEPROM.get(eeprom_mqtt_start + 80, mqtt_topic);
    EEPROM.get(eeprom_mqtt_start + 120, mqtt_enabled);
    EEPROM.get(eeprom_mqtt_start + 122, leds_enabled);
    EEPROM.get(eeprom_mqtt_start + 124, led_brightness);
    if (mqtt_enabled[0] != '0' && mqtt_enabled[0] != '1') mqtt_enabled[0] = '0';
    if (leds_enabled[0] != '0' && leds_enabled[0] != '1') leds_enabled[0] = '1';
}

void NetworkManager::saveSettings() {
    EEPROM.put(eeprom_mqtt_start, mqtt_server);
    EEPROM.put(eeprom_mqtt_start + 40, mqtt_user);
    EEPROM.put(eeprom_mqtt_start + 60, mqtt_pass);
    EEPROM.put(eeprom_mqtt_start + 80, mqtt_topic);
    EEPROM.put(eeprom_mqtt_start + 120, mqtt_enabled);
    EEPROM.put(eeprom_mqtt_start + 122, leds_enabled);
    EEPROM.put(eeprom_mqtt_start + 124, led_brightness);
    EEPROM.commit();
}

void NetworkManager::updateSettings(const char* s, const char* u, const char* p, const char* t, bool e, bool ledEnable, uint8_t brightness) {
    strncpy(mqtt_server, s, 39);
    strncpy(mqtt_user, u, 19);
    strncpy(mqtt_pass, p, 19);
    strncpy(mqtt_topic, t, 39);
    mqtt_enabled[0] = e ? '1' : '0';
    leds_enabled[0] = ledEnable ? '1' : '0';
    led_brightness = brightness;
    saveSettings();
    mqttClient.disconnect();
    mqttClient.setServer(mqtt_server, 1883);
}

bool NetworkManager::testConnection(const char* s, const char* u, const char* p, const char* t) {
    Serial.printf("MQTT Test: server=%s, user=%s\n", s, u);
    mqttClient.disconnect();
    mqttClient.setServer(s, 1883);
    
    bool connected = false;
    if (strlen(u) > 0) {
        connected = mqttClient.connect("BalancaTest", u, p);
    } else {
        connected = mqttClient.connect("BalancaTest");
    }

    if (connected) {
        String testTopic = String(t) + "/test";
        mqttClient.publish(testTopic.c_str(), "Teste de Conexão OK");
        mqttClient.disconnect();
        mqttClient.setServer(mqtt_server, 1883);
        return true;
    }
    mqttClient.setServer(mqtt_server, 1883);
    return false;
}

void NetworkManager::setupConfigPortal() {
    WiFiManager wm;
    wm.setAPStaticIPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    WiFiManagerParameter custom_mqtt_server("server", "Servidor MQTT", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_user("user", "Usuario MQTT", mqtt_user, 20);
    WiFiManagerParameter custom_mqtt_pass("pass", "Senha MQTT", mqtt_pass, 20);
    WiFiManagerParameter custom_mqtt_topic("topic", "Topico Base", mqtt_topic, 40);
    const char* mqtt_html = mqtt_enabled[0] == '1' ? "type='checkbox' value='1' checked" : "type='checkbox' value='1'";
    WiFiManagerParameter custom_mqtt_en("mqtt_en", "Ativar MQTT", "1", 2, mqtt_html, WFM_LABEL_AFTER);
    const char* led_html = leds_enabled[0] == '1' ? "type='checkbox' value='1' checked" : "type='checkbox' value='1'";
    WiFiManagerParameter custom_led_en("led_en", "Ativar LEDs", "1", 2, led_html, WFM_LABEL_AFTER);
    wm.addParameter(&custom_mqtt_server); wm.addParameter(&custom_mqtt_user); wm.addParameter(&custom_mqtt_pass); wm.addParameter(&custom_mqtt_topic); wm.addParameter(&custom_mqtt_en); wm.addParameter(&custom_led_en);
    if (!wm.startConfigPortal("BalancaAP")) { delay(1000); ESP.restart(); }
    strcpy(mqtt_server, custom_mqtt_server.getValue()); strcpy(mqtt_user, custom_mqtt_user.getValue()); strcpy(mqtt_pass, custom_mqtt_pass.getValue()); strcpy(mqtt_topic, custom_mqtt_topic.getValue());
    strcpy(mqtt_enabled, strlen(custom_mqtt_en.getValue()) == 0 ? "0" : "1");
    strcpy(leds_enabled, strlen(custom_led_en.getValue()) == 0 ? "0" : "1");
    saveSettings(); ESP.restart();
}

void NetworkManager::publishStatus(const char* status) {
    if (mqttClient.connected()) {
        String topic = String(mqtt_topic) + "/status";
        String payload = "{\"status\":\"" + String(status) + "\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
        mqttClient.publish(topic.c_str(), payload.c_str(), true);
    }
}

void NetworkManager::publishRecipe(const char* recipe) {
    if (mqttClient.connected()) {
        String topic = String(mqtt_topic) + "/receita";
        mqttClient.publish(topic.c_str(), recipe, true);
    }
}

void NetworkManager::publishWeight(float weight) {
    if (mqttClient.connected()) {
        String topic = String(mqtt_topic) + "/peso";
        char buf[12]; itoa((int)round(weight), buf, 10);
        mqttClient.publish(topic.c_str(), buf);
    }
}

bool NetworkManager::isConnected() { return WiFi.status() == WL_CONNECTED; }

void NetworkManager::loop() {
    if (isConnected() && mqtt_enabled[0] == '1') {
        if (!mqttClient.connected()) {
            static unsigned long lastMqttRetry = 0;
            if (millis() - lastMqttRetry > 15000) {
                lastMqttRetry = millis();
                bool connected = mqtt_user[0] != '\0' ? mqttClient.connect("BalancaIoT", mqtt_user, mqtt_pass) : mqttClient.connect("BalancaIoT");
                if (connected) publishStatus("online");
            }
        }
        mqttClient.loop();
    }
}
