#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <ESP8266WebServer.h>
#include "LoadCellMonitor.h"
#include "NetworkManager.h"

class WebServerManager {
private:
    ESP8266WebServer server;
    LoadCellMonitor& monitor;
    NetworkManager& network;

public:
    WebServerManager(uint16_t port, LoadCellMonitor& lm, NetworkManager& nm) 
        : server(port), monitor(lm), network(nm) {}

    void begin() {
        server.on("/", [this](){
            String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>body{font-family:sans-serif;background:#121212;color:white;display:flex;flex-direction:column;align-items:center;min-height:100vh;margin:0;padding:1rem;box-sizing:border-box}"
                ".card{background:#1e1e1e;padding:1.5rem;border-radius:15px;box-shadow:0 10px 30px rgba(0,0,0,0.5);text-align:center;width:100%;max-width:400px;position:relative}"
                ".weight{font-size:5rem;font-weight:bold;color:#00ff88;margin:0.5rem 0}"
                ".time{font-size:2rem;color:#00ccff;font-family:monospace;margin-bottom:1rem}"
                "select, button{background:#333;color:white;border:1px solid #444;padding:12px;border-radius:8px;width:100%;margin-top:10px;font-size:1rem;cursor:pointer;transition:0.2s}"
                "button.tare{background:#0066ff;font-weight:bold} button:active{transform:scale(0.98)}"
                ".gear-btn{position:absolute;top:15px;right:15px;background:none;border:none;color:#666;cursor:pointer;width:30px;height:30px;padding:0}"
                ".gear-btn svg{width:24px;height:24px}"
                ".modal{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.8);backdrop-filter:blur(5px);z-index:100;align-items:center;justify-content:center}"
                ".modal-content{background:#1e1e1e;padding:2rem;border-radius:15px;width:90%;max-width:350px;text-align:left;box-shadow:0 20px 50px rgba(0,0,0,0.8)}"
                ".modal-content h2{margin-top:0;font-size:1.2rem;color:#00ccff} label{font-size:0.8rem;color:#888;display:block;margin-top:10px}"
                "input[type=text], input[type=password]{background:#2a2a2a;color:white;border:1px solid #444;padding:10px;width:100%;border-radius:5px;margin-top:5px;box-sizing:border-box}"
                ".modal-footer{display:flex;flex-wrap:wrap;gap:10px;margin-top:20px} .close-btn{flex:1;background:#444} .save-btn{flex:1;background:#00ff88;color:#000} .test-btn{width:100%;background:#ffcc00;color:#000}</style></head><body>"
                "<div class='card'><button class='gear-btn' onclick='openModal()'><svg viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'><circle cx='12' cy='12' r='3'></circle><path d='M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z'></path></svg></button>"
                "<h1>BALANÇA IOT</h1><div id='time' class='time'>00:00.00</div><div id='weight' class='weight'>0<small>g</small></div>"
                "<button class='tare' onclick='doTare()'>TARA / RESET</button>"
                "<select id='recipeSelect' onchange='setRecipe(this.value)'><option value='Receita 1'>Receita 1</option><option value='Receita 2'>Receita 2</option><option value='Receita 3'>Receita 3</option></select></div>"
                "<div id='settingsModal' class='modal'><div class='modal-content'><h2>Configuração MQTT</h2>"
                "<label>Servidor</label><input id='mq_srv' type='text'><label>Usuário</label><input id='mq_usr' type='text'><label>Senha</label><input id='mq_pass' type='password'><label>Tópico Base</label><input id='mq_top' type='text'>"
                "<label style='display:flex;align-items:center;gap:10px;margin-top:15px'><input type='checkbox' id='mq_en'> Ativar MQTT</label>"
                "<div class='modal-footer'><button class='test-btn' id='testBtn' onclick='testMqtt()'>Testar Conexão</button><button class='close-btn' onclick='closeModal()'>Cancelar</button><button class='save-btn' onclick='saveMqtt()'>Salvar</button></div></div></div>"
                "<script>function doTare(){fetch('/tare')} function setRecipe(v){fetch('/setRecipe?name='+encodeURIComponent(v))}"
                "function openModal(){fetch('/getMqttConfig').then(r=>r.json()).then(d=>{document.getElementById('mq_srv').value=d.srv;document.getElementById('mq_usr').value=d.usr;document.getElementById('mq_pass').value=d.pas;document.getElementById('mq_top').value=d.top;document.getElementById('mq_en').checked=d.en=='1';document.getElementById('settingsModal').style.display='flex';})}"
                "function closeModal(){document.getElementById('settingsModal').style.display='none'}"
                "function testMqtt(){const b=document.getElementById('testBtn');b.innerText='Testando...';const s=document.getElementById('mq_srv').value,u=document.getElementById('mq_usr').value,p=document.getElementById('mq_pass').value,t=document.getElementById('mq_top').value;"
                "fetch(`/testMqtt?srv=${s}&usr=${u}&pas=${p}&top=${t}`).then(r=>r.text()).then(t=>{alert(t=='OK'?'Conectado com Sucesso!':'Falha na Conexão');b.innerText='Testar Conexão';})}"
                "function saveMqtt(){const s=document.getElementById('mq_srv').value,u=document.getElementById('mq_usr').value,p=document.getElementById('mq_pass').value,t=document.getElementById('mq_top').value,e=document.getElementById('mq_en').checked?'1':'0';"
                "fetch(`/saveMqtt?srv=${s}&usr=${u}&pas=${p}&top=${t}&en=${e}`).then(()=>{alert('Salvo!');closeModal();})}"
                "setInterval(function(){fetch('/data').then(r=>r.json()).then(data=>{document.getElementById('weight').innerHTML=Math.round(data.w)+'<small>g</small>';"
                "document.getElementById('time').innerText=data.t;document.getElementById('recipeSelect').value=data.r;});},250);</script></body></html>";
            server.send(200, "text/html", html);
        });

        server.on("/getMqttConfig", [this](){
            String json = "{\"srv\":\"" + String(network.getServer()) + "\",\"usr\":\"" + String(network.getUser()) + "\",\"pas\":\"" + String(network.getPass()) + "\",\"top\":\"" + String(network.getTopic()) + "\",\"en\":\"" + String(network.isMqttEnabled()?"1":"0") + "\"}";
            server.send(200, "application/json", json);
        });

        server.on("/testMqtt", [this](){
            if (network.testConnection(server.arg("srv").c_str(), server.arg("usr").c_str(), server.arg("pas").c_str(), server.arg("top").c_str())) {
                server.send(200, "text/plain", "OK");
            } else {
                server.send(200, "text/plain", "FAIL");
            }
        });

        server.on("/data", [this](){
            unsigned long t = monitor.getElapsedTime();
            char timeBuf[12];
            sprintf(timeBuf, "%02u:%02u.%02u", (unsigned int)((t/1000)/60), (unsigned int)((t/1000)%60), (unsigned int)((t%1000)/10));
            String json = "{\"w\":" + String(monitor.getWeight()) + ",\"t\":\"" + String(timeBuf) + "\",\"r\":\"" + String(monitor.getRecipe()) + "\"}";
            server.send(200, "application/json", json);
        });

        server.on("/tare", [this](){ monitor.tare(); server.send(200, "text/plain", "OK"); });

        server.on("/saveMqtt", [this](){
            network.updateSettings(server.arg("srv").c_str(), server.arg("usr").c_str(), server.arg("pas").c_str(), server.arg("top").c_str(), server.arg("en")=="1");
            server.send(200, "text/plain", "OK");
        });

        server.begin();
    }

    void stop() { server.stop(); server.close(); }
    void handle() { server.handleClient(); }
};

#endif
