
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
  if (CheckArrayList(TempIngoingStruct) >= 0) {
    //Serial.println("Peer already registered");

    for (int i = 0; i < 10; i++) {
      int IDIndexMatch = CheckArrayList(TempIngoingStruct);
      if (IDIndexMatch >= 0) {
        memcpy(&Peers[IDIndexMatch].IngoingStruct, incomingData, sizeof(Peers[IDIndexMatch].IngoingStruct));
        break;
      }
    }

    //Serial.print("Bytes received: ");
    //Serial.println(len);
  } else {
    Serial.println("who tf are you, get in here");
    registerPeers(TempIngoingStruct);
  }
}
// funktion to check arraylist if any of its arrays contain the incomeing macaddress
int CheckArrayList(struct Sensordata MacToCheck) {
  for (int i = 0; i < 10; i++) {
    if (memcmp(MacToCheck.id, Peers[i].peerInfo.peer_addr, sizeof(Peers[i].peerInfo.peer_addr)) == 0) {
      return i;
    }
  }
  return -1;
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
    if (memcmp(&Peers[j].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(Peers[j].peerInfo.peer_addr)) == 0) {
      tempIndex = j;
      break;
    } else {
      continue;
    }
  }
  memcpy(&Peers[tempIndex].peerInfo.peer_addr, MacToAdd.id, sizeof(Peers[tempIndex].peerInfo.peer_addr));


  Peers[tempIndex].peerInfo.channel = 0;
  Peers[tempIndex].peerInfo.encrypt = false;

  Serial.println("Peer MAC:");
  AddressOfPeer(Peers[tempIndex].peerInfo.peer_addr);

  if (esp_now_add_peer(&Peers[tempIndex].peerInfo) == ESP_OK) {
    Serial.println("Peer added successfully");
    Peers[tempIndex].isActive = true;
    TempIngoingStruct.command = 'C';  //command for connection confirmed
    esp_now_send(Peers[tempIndex].peerInfo.peer_addr, (uint8_t*)&TempIngoingStruct, sizeof(TempIngoingStruct));
  } else {
    Serial.print("Failed to add peer Error code: ");
    Serial.println(esp_now_add_peer(&Peers[tempIndex].peerInfo));
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
  tft.print(AveragesStruct.temp, 1);  // 1 decimal place
  tft.println(" C");

  tft.setCursor(0, 60);
  tft.print("Hum: ");
  tft.print(AveragesStruct.hum, 1);
  tft.println(" %");

  tft.setCursor(0, 90);
  tft.print("CO2: ");
  tft.print(AveragesStruct.co2, 1);
  tft.println(" ppm");

  tft.setCursor(0, 120);
  tft.print("Connected Peers: ");
  tft.print(AveragesStruct.activePeersTotal, 1);

  // Serial output remains the same
  Serial.println("INCOMING READINGS");
  Serial.print("Temperature: ");
  Serial.print(AveragesStruct.temp);
  Serial.println(" ºC");
  Serial.print("Humidity: ");
  Serial.print(AveragesStruct.hum);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(AveragesStruct.co2);
  Serial.println(" ppm");
  Serial.println();

  /*
  for (int i = 0; i < 6; i++) {
    if (TempIngoingStruct.id[i] < 16) { Serial.print("0"); }  // zero pad
    Serial.print(TempIngoingStruct.id[i], HEX);
    if (i < 5) { Serial.print(":"); }
  }
  Serial.println();
  */
}

void SendCommandAllSlaves(char command) {
  TempIngoingStruct.ping = '1';
  TempIngoingStruct.command = command;  //command for sending the data confirmed

  for (int i = 0; i < 10; i++) {
    esp_now_send(Peers[i].peerInfo.peer_addr, (uint8_t*)&TempIngoingStruct, sizeof(TempIngoingStruct));
  }
}

void CalculateAvrg(Sensordata* resultStruct) {
  uint8_t EMPTY_MAC_ADDRESS[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  resultStruct->temp = 0;
  resultStruct->hum = 0;
  resultStruct->co2 = 0;
  resultStruct->activePeersTotal = 0;
  int activePeersTotal = 0;

  for (int i = 0; i < 10; i++) {
    if (Peers[i].isActive == true) {
      resultStruct->temp += Peers[i].IngoingStruct.temp;
      resultStruct->hum += Peers[i].IngoingStruct.hum;
      resultStruct->co2 += Peers[i].IngoingStruct.co2;
      activePeersTotal++;

      esp_now_del_peer(Peers[i].peerInfo.peer_addr);
      /*
      if (Peers[i].IngoingStruct.ping != '2') {
        TempIngoingStruct.command = 'R';
        esp_err_t sendErr = esp_now_send(Peers[i].peerInfo.peer_addr, (uint8_t*)&TempIngoingStruct, sizeof(TempIngoingStruct));

        if (sendErr != ESP_OK) {
          Serial.println("Warning: Failed to send reset command to unresponsive peer.");
        }

        esp_err_t delErr = esp_now_del_peer(Peers[i].peerInfo.peer_addr);

        if (delErr == ESP_OK) {
          Serial.println("Successfully removed peer from internal ESP-NOW list.");
        }

        memcpy(&Peers[i].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(EMPTY_MAC_ADDRESS));


        Serial.println("Removed the peer: ");
        String ownMacHex;
        for (int j = 0; j < 6; j++) {
          String hexPart = String(Peers[i].peerInfo.peer_addr[j], HEX);
          if (hexPart.length() < 2) hexPart = "0" + hexPart;  // zero-pad if needed
          ownMacHex += hexPart;
          if (i < 5) ownMacHex += ":";  // add ':' except after last byte
        }
        Serial.println(ownMacHex);
        Serial.println();


        Peers[i].isActive = false;
        activePeersTotal--;
      } */
    }
        
    }
    resultStruct->temp /= activePeersTotal;
    resultStruct->hum /= activePeersTotal;
    resultStruct->co2 /= activePeersTotal;
    resultStruct->activePeersTotal = activePeersTotal;
    Serial.println(activePeersTotal);
  }
