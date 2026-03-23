#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "LoadCellMonitor.h"
#include "NetworkManager.h"
#include "RecipeManager.h"
#include "Recipe.h"

// HTML em PROGMEM para economizar RAM no ESP8266
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>
<style>body{font-family:sans-serif;background:#121212;color:white;display:flex;flex-direction:column;align-items:center;min-height:100vh;margin:0;padding:1rem;box-sizing:border-box}
.card{background:#1e1e1e;padding:1.5rem;border-radius:15px;box-shadow:0 10px 30px rgba(0,0,0,0.5);text-align:center;width:100%;max-width:400px;position:relative;margin-bottom:1rem}
.weight{font-size:5rem;font-weight:bold;color:#00ff88;margin:0.5rem 0}
.time{font-size:2rem;color:#00ccff;font-family:monospace;margin-bottom:1rem}
select, button{background:#333;color:white;border:1px solid #444;padding:12px;border-radius:8px;width:100%;margin-top:10px;font-size:1rem;cursor:pointer;transition:0.2s}
button.tare{background:#0066ff;font-weight:bold} button:active{transform:scale(0.98)}
.gear-btn{position:absolute;top:15px;right:15px;background:none;border:none;color:#666;cursor:pointer;width:30px;height:30px;padding:0}
.admin-btn{background:#444;margin-top:10px} .gear-btn svg{width:24px;height:24px}
.modal{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.8);backdrop-filter:blur(5px);z-index:100;align-items:center;justify-content:center;overflow-y:auto}
.modal-content{background:#1e1e1e;padding:1.5rem;border-radius:15px;width:90%;max-width:450px;text-align:left;box-shadow:0 20px 50px rgba(0,0,0,0.8);margin: auto;}
.modal-content h2{margin-top:0;font-size:1.2rem;color:#00ccff} label{font-size:0.8rem;color:#888;display:block;margin-top:10px}
input[type='text'], input[type='password'], input[type='number'], select {
    background:#2a2a2a;color:white;border:1px solid #444;padding:10px;width:100%;border-radius:5px;margin-top:5px;box-sizing:border-box
}
.check-row{display:flex;align-items:center;gap:10px;margin-top:12px;cursor:pointer;width:100%;justify-content:flex-start} 
.check-row input[type='checkbox']{width:20px;height:20px;margin:0;cursor:pointer;flex-shrink:0}
.check-row span{font-size:0.9rem;color:#eee}
input[type='range'] {width:100%;margin-top:10px;cursor:pointer;accent-color:#00ff88;height:10px;background:#444;border-radius:5px;appearance:none}
input[type='range']::-webkit-slider-thumb {appearance:none;width:18px;height:18px;background:#00ff88;border-radius:50%;cursor:pointer}
.modal-footer{display:flex;flex-wrap:wrap;gap:10px;margin-top:20px} .close-btn{flex:1;background:#444} .save-btn{flex:1;background:#00ff88;color:#000}
.recipe-item{background:#2a2a2a;padding:10px;border-radius:8px;margin-top:10px;border-left:4px solid #00ccff;position:relative}
.recipe-summary{background:#333;padding:10px;border-radius:8px;margin-top:15px;display:flex;justify-content:space-around;font-weight:bold;color:#00ff88}
.progress-list{margin-top:20px;text-align:left;width:100%}
.progress-list h3{font-size:0.9rem;color:#888;margin-bottom:10px;text-transform:uppercase;letter-spacing:1px}
.progress-step{background:#2a2a2a;padding:10px;border-radius:8px;margin-bottom:8px;border-left:4px solid #444;transition:0.3s;opacity:0.5}
.progress-step.active{border-left-color:#00ff88;background:#333;opacity:1;box-shadow:0 0 15px rgba(0,255,136,0.1);transform:scale(1.02)}
.progress-step.done{border-left-color:#00ccff;opacity:0.3}
.step-info{display:flex;justify-content:space-between;font-size:0.85rem;margin-top:4px;color:#00ff88}
.step-controls{display:flex;flex-direction:column;gap:2px;position:absolute;right:10px;top:10px}
.step-controls button{width:30px;height:30px;padding:0;margin:0;font-size:0.8rem;background:#444}
.import-export{display:flex;gap:10px;margin-top:10px} .ie-btn{font-size:0.8rem;padding:8px;background:#555}</style></head><body>
<div class='card'><button class='gear-btn' onclick='openMqttModal()'><svg viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'><circle cx='12' cy='12' r='3'></circle><path d='M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z'></path></svg></button>
<h1>BALANÇA IOT</h1><div id='time' class='time'>00:00.00</div><div id='weight' class='weight'>0<small>g</small></div>
<button class='tare' onclick='doTare()'>TARA / RESET</button>
<select id='recipeSelect' onchange='setRecipe(this.value)'></select>
<div id='recipeProgress' class='progress-list'></div>
<button class='admin-btn' onclick='openRecipeAdmin()'>Administrar Receitas</button></div>

<div id='recipeAdminModal' class='modal'><div class='modal-content'><h2>Administrar Receitas</h2>
<div id='recipeList'></div><button onclick='editRecipe()'>+ Nova Receita</button>
<div class='import-export'><button class='ie-btn' onclick='exportRecipes()'>Exportar JSON</button>
<button class='ie-btn' onclick='document.getElementById("importFile").click()'>Importar JSON</button>
<input type='file' id='importFile' style='display:none' onchange='importRecipes(this)'></div>
<div class='modal-footer'><button class='close-btn' onclick='closeModal("recipeAdminModal")'>Fechar</button></div></div></div>

<div id='editRecipeModal' class='modal'><div class='modal-content'><h2>Editar Receita</h2>
<label>Nome da Receita</label><input id='rec_name' type='text'><div id='stepList'></div>
<div class='recipe-summary'><div>Tempo: <span id='totalTime'>0</span>s</div><div>Total: <span id='totalAmount'>0</span>ml</div></div>
<button onclick='addStep("POUR")'>+ Despejar Água</button><button onclick='addStep("WAIT")'>+ Aguardar</button>
<div class='modal-footer'><button class='close-btn' onclick='closeModal("editRecipeModal")'>Cancelar</button><button class='save-btn' onclick='saveRecipe()'>Salvar</button></div></div></div>

<div id='mqttModal' class='modal'><div class='modal-content'><h2>Configurações</h2>
<label>Servidor MQTT</label><input id='mq_srv' type='text'><label>Usuário</label><input id='mq_usr' type='text'><label>Senha</label><input id='mq_pass' type='password'><label>Tópico Base</label><input id='mq_top' type='text'>
<div style='margin-top:15px; border-top:1px solid #333; padding-top:10px'>
    <label class='check-row'><input type='checkbox' id='mq_en'> <span>Ativar MQTT</span></label>
    <label class='check-row'><input type='checkbox' id='led_en'> <span>Ativar LEDs de Feedback</span></label>
</div>
<div style='margin-top:20px'>
    <label style='display:flex; justify-content:space-between'><span>Brilho dos LEDs</span><span id='br_val' style='color:#00ff88'>100</span></label>
    <input type='range' id='led_br' min='0' max='255' value='100'>
</div>
<div class='modal-footer'><button class='close-btn' onclick='closeModal("mqttModal")'>Cancelar</button><button class='save-btn' onclick='saveMqtt()'>Salvar</button></div></div></div>

<script>let recipes=[];
function doTare(){fetch('/tare')}
function setRecipe(v){fetch('/setRecipe?name='+encodeURIComponent(v))}
function loadRecipes(){fetch('/getRecipes').then(r=>r.json()).then(d=>{recipes=d;const s=document.getElementById('recipeSelect');const current=s.value;s.innerHTML='';d.forEach(r=>{const o=document.createElement('option');o.value=r.name;o.innerText=r.name;if(r.name===current)o.selected=true;s.appendChild(o)});updateRecipeList()})}
function updateRecipeList(){const l=document.getElementById('recipeList');l.innerHTML='';recipes.forEach(r=>{const d=document.createElement('div');d.style.display='flex';d.style.justifyContent='space-between';d.style.marginBottom='10px';
d.innerHTML=`<span>${r.name}</span><div><button onclick='editRecipe("${r.name}")' style='width:auto;margin:0 5px;padding:5px'>E</button><button onclick='deleteRecipe("${r.name}")' style='width:auto;margin:0;padding:5px;background:#ff4444'>X</button></div>`;l.appendChild(d)})}
function openMqttModal(){fetch('/getMqttConfig').then(r=>r.json()).then(d=>{document.getElementById('mq_srv').value=d.srv;document.getElementById('mq_usr').value=d.usr;document.getElementById('mq_pass').value=d.pas;document.getElementById('mq_top').value=d.top;document.getElementById('mq_en').checked=d.en=='1';document.getElementById('led_en').checked=d.led=='1';document.getElementById('led_br').value=d.br;document.getElementById('br_val').innerText=d.br;document.getElementById('mqttModal').style.display='flex'})}
document.getElementById('led_br').oninput=function(){document.getElementById('br_val').innerText=this.value};
function openRecipeAdmin(){loadRecipes();document.getElementById('recipeAdminModal').style.display='flex'}
function closeModal(id){document.getElementById(id).style.display='none'}
function editRecipe(name){const m=document.getElementById('editRecipeModal');m.style.display='flex';document.getElementById('rec_name').value=name||'';const sl=document.getElementById('stepList');sl.innerHTML='';const r=recipes.find(x=>x.name===name)||{steps:[]};r.steps.forEach(s=>addStep(s.type,s.amount,s.time));updateSummary()}
function addStep(type,amount,time){const sl=document.getElementById('stepList');const d=document.createElement('div');d.className='recipe-item';d.dataset.type=type;
const labelStyle="display:block;font-size:0.7rem;color:#888;margin-bottom:2px";
d.innerHTML=`<strong>${type=='POUR'?'DESPEJAR':'AGUARDAR'}</strong><div class='step-controls'><button onclick='moveStep(this,"UP")'>▲</button><button onclick='moveStep(this,"DOWN")'>▼</button></div><div style='display:flex;gap:10px;margin-top:5px'>`+
(type=='POUR'?`<div><span style='${labelStyle}'>Quantidade (ml)</span><input type='number' placeholder='ml' value='${amount||0}' oninput='updateSummary()'></div>`:'')+
`<div><span style='${labelStyle}'>Tempo (seg)</span><input type='number' placeholder='seg' value='${time||0}' oninput='updateSummary()'></div>`+
`<button onclick='this.parentElement.parentElement.remove();updateSummary()' style='width:auto;background:none;border:none;color:#ff4444;align-self:flex-end;padding-bottom:10px'>X</button></div>`;sl.appendChild(d);updateSummary()}
function moveStep(btn,dir){const item=btn.parentElement.parentElement;const sl=document.getElementById('stepList');if(dir==='UP'&&item.previousElementSibling)sl.insertBefore(item,item.previousElementSibling);else if(dir==='DOWN'&&item.nextElementSibling)sl.insertBefore(item.nextElementSibling,item);updateSummary()}
function updateSummary(){let t=0,a=0;document.querySelectorAll('#stepList .recipe-item').forEach(d=>{const inputs=d.querySelectorAll('input');if(d.dataset.type=='POUR'){a+=parseInt(inputs[0].value)||0;t+=parseInt(inputs[1].value)||0}else{t+=parseInt(inputs[0].value)||0}});document.getElementById('totalTime').innerText=t;document.getElementById('totalAmount').innerText=a}
function saveRecipe(){const name=document.getElementById('rec_name').value;if(!name)return alert('Nome obrigatorio');const steps=[];document.querySelectorAll('#stepList .recipe-item').forEach(d=>{const inputs=d.querySelectorAll('input');steps.push({type:d.dataset.type,amount:d.dataset.type=='POUR'?parseInt(inputs[0].value):0,time:d.dataset.type=='POUR'?parseInt(inputs[1].value):parseInt(inputs[0].value)})});
fetch('/saveRecipe',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name,steps})}).then(()=>{closeModal('editRecipeModal');loadRecipes()})}
function deleteRecipe(name){if(confirm('Excluir?'))fetch('/deleteRecipe?name='+encodeURIComponent(name)).then(loadRecipes)}
function saveMqtt(){const s=document.getElementById('mq_srv').value,u=document.getElementById('mq_usr').value,p=document.getElementById('mq_pass').value,t=document.getElementById('mq_top').value,e=document.getElementById('mq_en').checked?'1':'0',l=document.getElementById('led_en').checked?'1':'0',b=document.getElementById('led_br').value;
fetch(`/saveMqtt?srv=${s}&usr=${u}&pas=${p}&top=${t}&en=${e}&led=${l}&br=${b}`).then(()=>{alert('Salvo!');closeModal('mqttModal');})};
function exportRecipes(){const blob=new Blob([JSON.stringify(recipes,null,2)],{type:'application/json'});const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download='receitas_balanca.json';a.click()}
function importRecipes(input){const file=input.files[0];if(!file)return;const reader=new FileReader();reader.onload=function(e){try{const data=JSON.parse(e.target.result);if(confirm('Isso ira sobrescrever suas receitas. Continuar?')){fetch('/saveRecipe',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({import:true,recipes:data})}).then(()=>{alert('Importado!');loadRecipes()})}}catch(err){alert('Arquivo invalido')}};reader.readAsText(file)}
function formatSecs(s){return Math.floor(s/60).toString().padStart(2,'0')+':'+(s%60).toString().padStart(2,'0')}
function timeToSecs(t){const parts=t.split(':');if(parts.length<2)return 0;const m=parseInt(parts[0]);const s=parseFloat(parts[1]);return (m*60)+s}
let lastRecipeName='';
setInterval(function(){fetch('/data').then(r=>r.json()).then(data=>{document.getElementById('weight').innerHTML=Math.round(data.w)+'<small>g</small>';document.getElementById('time').innerText=data.t;document.getElementById('recipeSelect').value=data.r;
const activeRec=recipes.find(r=>r.name===data.r);const progDiv=document.getElementById('recipeProgress');if(activeRec && activeRec.steps.length>0){if(lastRecipeName!==data.r){progDiv.innerHTML='<h3>Passos da Receita</h3>';activeRec.steps.forEach((s,i)=>{const d=document.createElement('div');d.className='progress-step';d.id='step-'+i;progDiv.appendChild(d)});lastRecipeName=data.r}
let accT=0,accW=0,foundActive=false;const currentSecs=timeToSecs(data.t);activeRec.steps.forEach((s,i)=>{accT+=s.time;accW+=s.amount;const el=document.getElementById('step-'+i);let state='';if(!foundActive && currentSecs < accT){state='active'; foundActive=true} else if(currentSecs >= accT){state='done'}
el.className='progress-step '+state;el.innerHTML=`<strong>${i+1}. ${s.type=='POUR'?'DESPEJAR':'AGUARDAR'}</strong><div class='step-info'><span>Alvo: ${accW}ml</span><span>Tempo: ${formatSecs(accT)}</span></div>`})}else{progDiv.innerHTML='';lastRecipeName=''}});},500);loadRecipes();</script></body></html>
)rawliteral";

class WebServerManager {
private:
    ESP8266WebServer server;
    LoadCellMonitor& monitor;
    NetworkManager& network;
    RecipeManager& recipeManager;

public:
    WebServerManager(uint16_t port, LoadCellMonitor& lm, NetworkManager& nm, RecipeManager& rm) 
        : server(port), monitor(lm), network(nm), recipeManager(rm) {}

    void begin() {
        server.on("/", [this](){ server.send_P(200, "text/html", INDEX_HTML); });
        server.on("/getRecipes", [this](){ server.send(200, "application/json", recipeManager.getRecipesJson()); });

        server.on("/saveRecipe", HTTP_POST, [this](){
            if (server.hasArg("plain")) {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, server.arg("plain"));
                if (error) { server.send(400, "text/plain", "Invalid JSON"); return; }
                if (doc.containsKey("import") && doc["import"].as<bool>()) {
                    JsonArray arr = doc["recipes"];
                    for (JsonObject rObj : arr) {
                        Recipe r; r.name = rObj["name"].as<String>();
                        JsonArray steps = rObj["steps"];
                        for (JsonObject sObj : steps) {
                            RecipeStep step;
                            step.type = (sObj["type"] == "POUR") ? RecipeStepType::POUR : RecipeStepType::WAIT;
                            step.amount = sObj["amount"] | 0; step.time = sObj["time"] | 0;
                            r.steps.push_back(step);
                        }
                        recipeManager.addOrUpdateRecipe(r);
                    }
                    server.send(200, "text/plain", "OK"); return;
                }
                Recipe r; r.name = doc["name"].as<String>();
                JsonArray steps = doc["steps"];
                for (JsonObject s : steps) {
                    RecipeStep step;
                    step.type = (s["type"] == "POUR") ? RecipeStepType::POUR : RecipeStepType::WAIT;
                    step.amount = s["amount"] | 0; step.time = s["time"] | 0;
                    r.steps.push_back(step);
                }
                recipeManager.addOrUpdateRecipe(r);
                server.send(200, "text/plain", "OK");
            }
        });

        server.on("/deleteRecipe", [this](){ recipeManager.deleteRecipe(server.arg("name")); server.send(200, "text/plain", "OK"); });

        server.on("/getMqttConfig", [this](){
            String json = "{\"srv\":\"" + String(network.getServer()) + "\",\"usr\":\"" + String(network.getUser()) + "\",\"pas\":\"" + String(network.getPass()) + "\",\"top\":\"" + String(network.getTopic()) + "\",\"en\":\"" + String(network.isMqttEnabled()?"1":"0") + "\",\"led\":\"" + String(network.isLedEnabled()?"1":"0") + "\",\"br\":\"" + String(network.getLedBrightness()) + "\"}";
            server.send(200, "application/json", json);
        });

        server.on("/data", [this](){
            unsigned long t = monitor.getElapsedTime(); char timeBuf[12];
            sprintf(timeBuf, "%02u:%02u.%02u", (unsigned int)((t/1000)/60), (unsigned int)((t/1000)%60), (unsigned int)((t%1000)/10));
            String json = "{\"w\":" + String(monitor.getWeight()) + ",\"t\":\"" + String(timeBuf) + "\",\"r\":\"" + String(monitor.getRecipe()) + "\"}";
            server.send(200, "application/json", json);
        });

        server.on("/tare", [this](){ monitor.tare(); server.send(200, "text/plain", "OK"); });
        
        server.on("/setRecipe", [this](){ 
            String name = server.arg("name"); const auto& recipes = recipeManager.getRecipes(); bool found = false;
            for (const auto& r : recipes) { if (r.name == name) { monitor.setRecipe(r); found = true; break; } }
            if (!found) { Recipe livre; livre.name = name; monitor.setRecipe(livre); }
            server.send(200, "text/plain", "OK"); 
        });

        server.on("/saveMqtt", [this](){
            network.updateSettings(server.arg("srv").c_str(), server.arg("usr").c_str(), server.arg("pas").c_str(), server.arg("top").c_str(), server.arg("en")=="1", server.arg("led")=="1", server.arg("br").toInt());
            server.send(200, "text/plain", "OK");
        });

        server.begin();
    }

    void handle() { server.handleClient(); }
    void stop() { server.stop(); server.close(); }
};

#endif
