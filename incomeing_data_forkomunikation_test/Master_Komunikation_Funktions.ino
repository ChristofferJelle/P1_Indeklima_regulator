uint8_t* readMacAddress() {
  static uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    return baseMac;
  } else {
    Serial.println("Failed to read MAC address");
    return nullptr;
  }
}
void InitDisplay() {
  //initialise lillygo screen
  tft.init();
  tft.setRotation(1);  // landscape mode
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);  // scale text
}

void InitESP32_NOW() {
  //initialises what kinda wifi mode its in, rn its in accese point mode, so i dont have to connect to uni wifi with own student password.
  WiFi.mode(WIFI_STA);

  //initatite ESP_now with library's own funktion for comunication without router.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  }
  //Register that i want to recive to cb. basically says "everytime i send data u just need to run this too"
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&IngoingStruct, incomingData, sizeof(IngoingStruct));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
}
