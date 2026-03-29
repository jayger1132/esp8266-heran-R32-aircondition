/*
 * IR Receiver - 紅外線訊號接收器
 * 
 * 用途: 接收冷氣遙控器的紅外線訊號，記錄 RAW DATA
 * 硬體: ESP8266 + IR Receiver (例如 VS1838B / TSOP1738)
 * 
 * 接線:
 *   IR Receiver OUT  -> GPIO14 (D5)
 *   IR Receiver VCC  -> 3.3V
 *   IR Receiver GND  -> GND
 * 
 * 使用方式:
 *   1. 上傳此程式到 ESP8266
 *   2. 開啟 Serial Monitor (115200 baud)
 *   3. 對準 IR Receiver 按下遙控器按鈕
 *   4. 記錄顯示的 RAW DATA 和解碼結果
 */

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

// ====== 設定 ======
const uint16_t kRecvPin   = 14;   // GPIO14 = D5，接 IR Receiver 的 OUT
const uint16_t kCaptureBufferSize = 4096;  // 拉到最大
const uint8_t  kTimeout   = 200;  // 拉到 200ms 確保完整

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

int signalCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(50);

  Serial.println();
  Serial.println("====================================");
  Serial.println("  ESP8266 IR Receiver - 紅外線接收器");
  Serial.println("====================================");
  Serial.println();
  Serial.println("IR Receiver Pin: GPIO14 (D5)");
  Serial.println("等待紅外線訊號...");
  Serial.println("請對準 IR Receiver 按下遙控器按鈕");
  Serial.println();
  Serial.println("------------------------------------");

  irrecv.enableIRIn();  // 啟動 IR 接收
}

void loop() {
  if (irrecv.decode(&results)) {
    signalCount++;
    
    Serial.println();
    Serial.print("========== 訊號 #");
    Serial.print(signalCount);
    Serial.println(" ==========");

    // 🚩 檢查是否有溢位
    if (results.overflow) {
      Serial.println("⚠️ 警告：緩衝區溢位！訊號可能不完整。");
      Serial.println("建議：請將 kCaptureBufferSize 調大 (例如 2048)。");
    }
    
    Serial.println();

    // 顯示協議類型
    Serial.print("協議 (Protocol): ");
    Serial.println(typeToString(results.decode_type));
    
    // 顯示位元數
    Serial.print("位元數 (Bits): ");
    Serial.println(results.bits);

    // 如果有解碼結果，顯示解碼值
    if (results.decode_type != decode_type_t::UNKNOWN) {
      Serial.print("解碼值 (Value): 0x");
      Serial.println(resultToHexidecimal(&results));
    }
    
    Serial.println();
    
    // 顯示 RAW DATA（最重要！）
    Serial.println("--- RAW DATA (可直接複製使用) ---");
    Serial.println();
    
    // 以 C 陣列格式輸出，方便直接貼到發射程式
    Serial.print("uint16_t rawData[");
    Serial.print(results.rawlen - 1);
    Serial.print("] = {");
    
    for (uint16_t i = 1; i < results.rawlen; i++) {
      if (i > 1) Serial.print(", ");
      if (i % 10 == 1) {
        Serial.println();
        Serial.print("    ");
      }
      Serial.print(results.rawbuf[i] * kRawTick);
    }
    Serial.println();
    Serial.println("};");
    
    Serial.println();
    Serial.println("--- RAW TIMING (詳細時序) ---");
    Serial.println(resultToSourceCode(&results));
    
    Serial.println("--- 人類可讀格式 ---");
    Serial.println(resultToHumanReadableBasic(&results));
    
    Serial.println("============================");
    Serial.println();
    Serial.println("等待下一個訊號...");
    
    irrecv.resume();  // 準備接收下一個訊號
  }
  yield();  // 讓 ESP8266 處理背景任務
}
