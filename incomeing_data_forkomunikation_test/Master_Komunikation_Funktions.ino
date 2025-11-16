
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
  memcpy(&TempIngoingStruct, incomingData, sizeof(TempIngoingStruct));

  // The condition reads: "If the first 6 bytes of peerInfo are NOT equal to the first 6 bytes of IngoingStruct.id"
  if (CheckArrayList(TempIngoingStruct) == true) {
    //Serial.println("Peer already registered");
    memcpy(&IngoingStruct, incomingData, sizeof(IngoingStruct));

    
    //Serial.print("Bytes received: ");
    //Serial.println(len);
  } else {
  }
}
// funktion to check arraylist if any of its arrays contain the incomeing macaddress
bool CheckArrayList(struct Sensordata MacToCheck) {
  for (int i = 0; i < 10; i++) {
    if (memcmp(MacToCheck.id, peerInfo[i].peer_addr, sizeof(peerInfo[i].peer_addr)) == 0) {
      return true;
    }
  }
  Serial.println("who tf are you, get in here");
  registerPeers(TempIngoingStruct);
  return false;
}

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

void registerPeers(struct Sensordata MacToAdd) {
  int tempIndex;
  uint8_t EMPTY_MAC_ADDRESS[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  // Register peer
  for (int j = 0; j < 10; j++) {
    if (memcmp(&peerInfo[j].peer_addr, EMPTY_MAC_ADDRESS, sizeof(peerInfo[j].peer_addr)) == 0) {
      tempIndex = j;
      break;
    } else {
      continue;
    }
  }
  memcpy(&peerInfo[tempIndex].peer_addr, MacToAdd.id, sizeof(peerInfo[tempIndex].peer_addr));


  peerInfo[tempIndex].channel = 0;
  peerInfo[tempIndex].encrypt = false;

  Serial.println("Peer MAC:");
  AddressOfPeer(peerInfo[tempIndex].peer_addr);

  if (esp_now_add_peer(&peerInfo[tempIndex]) == ESP_OK) {
    Serial.println("Peer added successfully");
    CommandStruct.command = 'C';  //command for connection confirmed
    esp_now_send(peerInfo[tempIndex].peer_addr, (uint8_t*)&CommandStruct, sizeof(CommandStruct));
  } else {
    Serial.print("Failed to add peer Error code: ");
    Serial.println(esp_now_add_peer(&peerInfo[tempIndex]));
  }
}

void AddressOfPeer(uint8_t peerInfoMAC[]) {
  for (int i = 0; i < 6; i++) {
    if (peerInfoMAC[i] < 16) { Serial.print("0"); }  // zero pad
    Serial.print(peerInfoMAC[i], HEX);
    if (i < 5) { Serial.print(":"); }
  }
}

void DrawDisplay() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("INCOMING READINGS");

  tft.setCursor(0, 30);
  tft.print("Temp: ");
  tft.print(IngoingStruct.temp, 1);  // 1 decimal place
  tft.println(" C");

  tft.setCursor(0, 60);
  tft.print("Hum: ");
  tft.print(IngoingStruct.hum, 1);
  tft.println(" %");

  tft.setCursor(0, 90);
  tft.print("co2: ");
  tft.print(IngoingStruct.co2, 1);
  tft.println(" ppm");

  // Serial output remains the same
  Serial.println("INCOMING READINGS");
  Serial.print("Temperature: ");
  Serial.print(IngoingStruct.temp);
  Serial.println(" ºC");
  Serial.print("Humidity: ");
  Serial.print(IngoingStruct.hum);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(IngoingStruct.co2);
  Serial.println(" ppm");
  Serial.println();
  for (int i = 0; i < 6; i++) {
    if (IngoingStruct.id[i] < 16) { Serial.print("0"); }  // zero pad
    Serial.print(IngoingStruct.id[i], HEX);
    if (i < 5) { Serial.print(":"); }
  }
  Serial.println();
}

void SendCommandAllSlaves(char command) {
  CommandStruct.command = command;  //command for sending the data confirmed
  for (int i = 0; i < 10; i++) {
    esp_now_send(peerInfo[i].peer_addr, (uint8_t*)&CommandStruct, sizeof(CommandStruct));
  }
}
