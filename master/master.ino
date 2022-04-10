#include <M5StickC.h>
#include <esp_now.h>
#include <WiFi.h>
esp_now_peer_info_t slave;

#define CMD_KASOKUDO 150

float gyroX, gyroY, gyroZ;  // ジャイロデータ取得　→回転(左90 0/-50/0, 右90 0/50/0)
float ax, ay, az;  // 加速度データ取得　→傾き(X/Y/Z: 正 0/0/1, 左90 1/0/0, 右90 -1/0/0, 上90 0/-1/0, 下90 0/1/0)

void setup() {
  M5.begin();
  M5.MPU6886.Init();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);
  // ESP-NOW初期化
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  } else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
  // マルチキャスト用Slave登録
  memset(&slave, 0, sizeof(slave));
  for (int i = 0; i < 6; ++i) {
    slave.peer_addr[i] = (uint8_t)0xff;
  }
  esp_err_t addStatus = esp_now_add_peer(&slave);
  if (addStatus == ESP_OK) {
    // Pair success
    Serial.println("Pair success");
  }
}

void loop() {
  M5.MPU6886.getGyroData(&gyroX, &gyroY, &gyroZ);
  M5.MPU6886.getAccelData(&ax, &ay, &az);
  M5.Lcd.setCursor(0, 30);

  M5.Lcd.printf("X:%7.2f\nY:%7.2f\nZ:%7.2f mg", gyroX * M5.MPU6886.gRes, gyroY * M5.MPU6886.gRes, gyroZ * M5.MPU6886.gRes);
  M5.Lcd.printf("X:%7.2f\nY:%7.2f\nZ:%7.2f mg", ax , ay , az );

  int active = 0;
  if(gyroX * M5.MPU6886.gRes > 10 || gyroX * M5.MPU6886.gRes < -10)
  {
    active = 1;
    Serial.println("active");
  }
  else
  {
    active = 0;
    Serial.println("deactive");
  }
  uint8_t data[2] = {CMD_KASOKUDO, active};
  esp_err_t result = esp_now_send(slave.peer_addr, data, sizeof(data));
  
  delay(1500);
}
