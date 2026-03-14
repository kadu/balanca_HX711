#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <ESP8266WebServer.h>
#include "LoadCellMonitor.h"

class WebServerManager {
private:
    ESP8266WebServer server;
    LoadCellMonitor& monitor;

public:
    WebServerManager(uint16_t port, LoadCellMonitor& lm) 
        : server(port), monitor(lm) {}

    void begin() {
        // Rota principal: HTML com AJAX para tempo real
        server.on("/", [this](){
            String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>body{font-family:sans-serif;background:#121212;color:white;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;margin:0}"
                ".card{background:#1e1e1e;padding:2rem;border-radius:15px;box-shadow:0 10px 30px rgba(0,0,0,0.5);text-align:center;width:90%;max-width:400px}"
                ".weight{font-size:5rem;font-weight:bold;color:#00ff88;margin:1rem 0}"
                ".time{font-size:2rem;color:#00ccff;font-family:monospace}"
                ".recipe{font-size:1.2rem;color:#aaa;margin-bottom:1rem;text-transform:uppercase;letter-spacing:2px}"
                "h1{margin:0;font-size:1rem;color:#666}</style></head><body>"
                "<div class='card'><h1>BALANCA IOT (SYNC)</h1><div id='recipe' class='recipe'>---</div>"
                "<div id='time' class='time'>00:00.00</div><div id='weight' class='weight'>0<small>g</small></div></div>"
                "<script>setInterval(function(){"
                "fetch('/data').then(response => response.json()).then(data => {"
                "document.getElementById('weight').innerHTML = Math.round(data.w) + '<small>g</small>';"
                "document.getElementById('time').innerText = data.t;"
                "document.getElementById('recipe').innerText = data.r;"
                "});}, 200);</script></body></html>";
            server.send(200, "text/html", html);
        });

        // Rota de dados (JSON)
        server.on("/data", [this](){
            unsigned long t = monitor.getElapsedTime();
            unsigned int mins = (t / 1000) / 60;
            unsigned int secs = (t / 1000) % 60;
            unsigned int cents = (t % 1000) / 10;
            char timeBuf[12];
            sprintf(timeBuf, "%02u:%02u.%02u", mins, secs, cents);

            String json = "{\"w\":" + String(monitor.getWeight()) + 
                          ",\"t\":\"" + String(timeBuf) + 
                          "\",\"r\":\"" + String(monitor.getRecipe()) + "\"}";
            server.send(200, "application/json", json);
        });

        server.begin();
        Serial.println("Web Server Sincrono iniciado!");
    }

    void stop() {
        server.stop();
        server.close();
        Serial.println("Web Server parado para configuracao.");
    }

    void handle() {
        server.handleClient();
    }
};

#endif
