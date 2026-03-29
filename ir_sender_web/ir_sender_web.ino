/*
 * IR Sender Web Controller - 禾聯冷氣網頁遙控器 (精確代碼版)
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <IRsend.h>

// ====== WiFi 設定 ======
const char* WIFI_SSID     = "wifizhong-2F-2.4G";
const char* WIFI_PASSWORD = "22928256";

// ====== 硬體設定 ======
const uint16_t kIrLedPin = 4;  // GPIO4 = D2
IRsend irsend(kIrLedPin);
ESP8266WebServer server(80);

// ====== COOLIX 代碼定義 ======
const uint32_t IR_OFF         = 0xB27BE0;
const uint32_t IR_ON_23       = 0xB21F58;  // 開機且 23度
const uint32_t IR_TEMP_24     = 0xB21F48;
const uint32_t IR_TEMP_25     = 0xB21FC8;
const uint32_t IR_TEMP_26     = 0xB21FD8;
const uint32_t IR_SELF_CLEAN  = 0xB5F5AA;

// ====== 網頁 HTML ======
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-TW">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>禾聯冷氣遙控器</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Noto+Sans+TC:wght@400;500&display=swap');
    body { font-family: 'Noto Sans TC', sans-serif; background: #0c1220; color: white; display: flex; flex-direction: column; align-items: center; padding: 20px; }
    .card { background: rgba(255,255,255,0.05); backdrop-filter: blur(10px); border-radius: 20px; padding: 20px; width: 100%; max-width: 350px; border: 1px solid rgba(255,255,255,0.1); }
    .btn { width: 100%; padding: 18px; margin: 8px 0; font-size: 1rem; border: none; border-radius: 12px; cursor: pointer; transition: 0.2s; font-weight: 500; }
    .btn-on { background: linear-gradient(135deg, #3b82f6, #6366f1); color: white; }
    .btn-off { background: #ef4444; color: white; }
    .btn-temp { background: rgba(255,255,255,0.1); color: #a8b4ff; border: 1px solid rgba(168, 180, 255, 0.2); }
    .btn-clean { background: rgba(74, 222, 128, 0.1); color: #4ade80; border: 1px solid rgba(74, 222, 128, 0.2); }
    .btn:active { transform: scale(0.96); opacity: 0.8; }
    #msg { height: 20px; margin-top: 15px; color: #4ade80; font-size: 0.9rem; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
  </style>
</head>
<body>
  <div class="card">
    <h2 style="text-align:center; margin-bottom:20px; color:#a8b4ff;">HERAN 冷氣控制</h2>
    <button class="btn btn-on" onclick="send('on')">⚡ 開機 (23°C)</button>
    <button class="btn btn-off" onclick="send('off')">⏻ 關機</button>
    
    <div style="margin: 20px 0 10px 0; color: rgba(255,255,255,0.4); font-size: 0.8rem; letter-spacing: 2px;">溫度設定</div>
    <div class="grid">
      <button class="btn btn-temp" onclick="send('t24')">24°C</button>
      <button class="btn btn-temp" onclick="send('t25')">25°C</button>
      <button class="btn btn-temp" onclick="send('t26')" style="grid-column: span 2;">26°C</button>
    </div>

    <div style="margin: 20px 0 10px 0; color: rgba(255,255,255,0.4); font-size: 0.8rem; letter-spacing: 2px;">特殊功能</div>
    <button class="btn btn-clean" onclick="send('clean')">✨ 自潔淨功能</button>
    
    <div id="msg" style="text-align:center;"></div>
  </div>

  <script>
    function send(cmd) {
      const fb = document.getElementById('msg');
      fb.innerText = '正在發送指令...';
      fetch('/ir?cmd=' + cmd).then(r => r.text()).then(t => {
        fb.innerText = '✅ 訊號已發送';
        setTimeout(() => fb.innerText = '', 2000);
      });
    }
  </script>
</body>
</html>
)rawliteral";

void handleIR() {
  String cmd = server.arg("cmd");
  uint32_t code = 0;

  if (cmd == "on")     code = IR_ON_23;
  else if (cmd == "off")    code = IR_OFF;
  else if (cmd == "t24")    code = IR_TEMP_24;
  else if (cmd == "t25")    code = IR_TEMP_25;
  else if (cmd == "t26")    code = IR_TEMP_26;
  else if (cmd == "clean")  code = IR_SELF_CLEAN;

  if (code > 0) {
    irsend.sendCOOLIX(code, 24);
    server.send(200, "text/plain", "OK");
    Serial.printf("發送指令: %s (0x%06X)\n", cmd.c_str(), code);
  } else {
    server.send(400, "text/plain", "Bad Cmd");
  }
}

void setup() {
  Serial.begin(115200);
  irsend.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nIP: " + WiFi.localIP().toString());
  server.on("/", []() { server.send(200, "text/html", HTML_PAGE); });
  server.on("/ir", handleIR);
  server.begin();
}

void loop() { server.handleClient(); }
