void InitESP() {
  pinMode(BUTTON_PIN, INPUT);

  //initialise lilygo screen:
  tft.init();
  tft.setRotation(1); //landscape mode
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2); //scale text

  //initialise + setup ESP_NOW library:
  WiFi.mode(WIFI_STA); //access point mode to get around using 'actual' wifi
  if (esp_now_init() != ESP_OK) { //initialise ESP-NOW for communication without router
    Serial.println("Error initialising ESP-NOW");
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataReceived)); //every time data is received, OnDataReceived() is called
}

void InitRotaryEncoder() {
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  attachInterrupt(CLK_PIN, InterruptCallback, FALLING);

  prevButtonSate = digitalRead(SW_PIN);

  //read initial state of CLK
  lastStateCLK = digitalRead(CLK_PIN);
  currentStateCLK = digitalRead(CLK_PIN);
}

void InitServo() {
  servo.attach(SERVO_PIN);
  ServoClose();

  pinMode(SHUNT_PIN, INPUT);
}

//get own MAC-address
uint8_t* ReadOwnMacAddress() {
  static uint8_t baseMac[6];

  if (esp_wifi_get_mac(WIFI_IF_STA, baseMac) == ESP_OK) {
    return baseMac; //ptr is returned
  } else {
    Serial.println("Failed to read MAC-address.");
    return nullptr;
  }
}
