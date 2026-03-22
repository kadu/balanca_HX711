#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <ArduinoOTA.h>
#include "DisplayManager.h"
#include "LoadCellMonitor.h"

class OTAManager {
private:
    DisplayManager& display;
    LoadCellMonitor& monitor;
    int lastPercentage = -1;

public:
    OTAManager(DisplayManager& dm, LoadCellMonitor& lm) : display(dm), monitor(lm) {}

    void begin(const char* hostname = "BalancaIoT") {
        ArduinoOTA.setHostname(hostname);

        ArduinoOTA.onStart([this]() {
            // CRÍTICO: Para a balança para liberar a CPU e evitar crash de rede
            monitor.stop();
            
            String type = (ArduinoOTA.getCommand() == U_FLASH) ? "Sketch" : "Filesystem";
            display.showStatus("OTA UPDATE", "Iniciando...", type.c_str());
            Serial.println("OTA: Start updating " + type);
        });

        ArduinoOTA.onEnd([this]() {
            display.showStatus("OTA UPDATE", "Sucesso!", "Reiniciando...");
            Serial.println("\nOTA: End Success");
        });

        ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
            int percentage = progress / (total / 100);
            
            // Só atualiza o OLED a cada 5% para não travar a rede com tráfego I2C
            if (percentage % 5 == 0 && percentage != lastPercentage) {
                lastPercentage = percentage;
                char buf[32];
                sprintf(buf, "Progresso: %d%%", percentage);
                display.showStatus("OTA UPDATE", "Baixando...", buf);
                Serial.printf("Progress: %u%%\r", percentage);
            }
            yield();
        });

        ArduinoOTA.onError([this](ota_error_t error) {
            const char* errStr = "Erro Desconhecido";
            if (error == OTA_AUTH_ERROR) errStr = "Erro Autenticacao";
            else if (error == OTA_BEGIN_ERROR) errStr = "Erro de Inicio";
            else if (error == OTA_CONNECT_ERROR) errStr = "Erro de Conexao";
            else if (error == OTA_RECEIVE_ERROR) errStr = "Erro de Recebimento";
            else if (error == OTA_END_ERROR) errStr = "Erro de Fim";
            
            display.showStatus("OTA ERRO", errStr, "Tente USB");
            Serial.printf("Error[%u]: %s\n", error, errStr);
        });

        ArduinoOTA.begin();
    }

    void handle() {
        ArduinoOTA.handle();
    }
};

#endif
