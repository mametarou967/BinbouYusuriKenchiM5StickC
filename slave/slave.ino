#include <M5StickC.h>
#include <esp_now.h>
#include <WiFi.h>
esp_now_peer_info_t slave;

#define CMD_KASOKUDO 150

#define motor_pin  32
int freq = 10000;
int ledChannel = 0;
int resolution = 10;

bool activeStatus = false;
portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

// 受信コールバック
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if(data[0] == CMD_KASOKUDO)
  {

    portENTER_CRITICAL_ISR(&mutex);
    if(data[1] == 1){
      activeStatus = true;
      // Serial.print("rcv true");
    }else{
      activeStatus = false;
      // Serial.print("rcv false");
    }
    portEXIT_CRITICAL_ISR(&mutex);
  }
}
void setup() {
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(3);

  ledcSetup(ledChannel, freq, resolution);  //Sets the frequency and number of counts corresponding to the channel.  设置通道对应的频率和计数位数
  ledcAttachPin(motor_pin, ledChannel); //Binds the specified channel to the specified I/O port for output.  将指定通道绑定到指定 IO 口上以实现输出
  
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
  // ESP-NOWコールバック登録
  esp_now_register_recv_cb(OnDataRecv);
}
void loop() {
  M5.update();

  if(activeStatus){
    ledcWrite(ledChannel, 128);  //Output PWM.  输出PWM

    portENTER_CRITICAL_ISR(&mutex);
    activeStatus = false;
    portEXIT_CRITICAL_ISR(&mutex);
  }else{
    ledcWrite(ledChannel, 0);
  }
  
  delay(500);
}
