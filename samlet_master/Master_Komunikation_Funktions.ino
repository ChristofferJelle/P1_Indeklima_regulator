void InitDisplay() {
  //initialise lillygo screen
  tft.init();
  tft.setRotation(1);  // landscape mode
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);  // scale text
}

//initialise everything that has to do with setting up ESPNOW library
void InitESP32_NOW() {
  //initialises what kinda wifi mode its in, rn its in accese point mode, so i dont have to connect to uni wifi with own student password.
  WiFi.mode(WIFI_STA);

  //initatite ESP_now with library's own funktion for comunication without router.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  }
  //Register that i want to recive to cb. basically says "everytime i send data u just need to run this too"
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataReceived));
}

// Callback when data is received
void OnDataReceived(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&TempIngoingStruct, incomingData, sizeof(TempIngoingStruct));
  // The condition reads: "If the first 6 bytes of peerInfo are NOT equal to the first 6 bytes of IngoingStruct.id"
  if (CheckArrayList(TempIngoingStruct) >= 0) {
    //Serial.println("Peer already registered");

    for (int i = 0; i < 10; i++) {
      int IDIndexMatch = CheckArrayList(TempIngoingStruct);
      if (IDIndexMatch >= 0) {
        memcpy(&Peers[IDIndexMatch].IngoingStruct, incomingData, sizeof(Peers[IDIndexMatch].IngoingStruct));
        // Update peer last seen time
        Peers[IDIndexMatch].lastSeenTime = millis();
        break;
      }
    }

    //Serial.print("Bytes received: ");
    //Serial.println(len);
  } else {
    Serial.println("who tf are you, get in here");
    RegisterPeers(TempIngoingStruct);
  }
}

// funktion to check arraylist if any of its arrays contain the incomeing macaddress
int CheckArrayList(struct SensordataTp MacToCheck) {
  for (int i = 0; i < 10; i++) {
    if (memcmp(MacToCheck.id, Peers[i].peerInfo.peer_addr, sizeof(Peers[i].peerInfo.peer_addr)) == 0) {
      return i;
    }
  }
  return -1;
}

//this funktion just uses a funktion to get the mac address and then returns it....and checks if it actually got it
uint8_t* ReadMacAddress() {
  static uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    return baseMac;
  } else {
    Serial.println("Failed to read MAC address");
    return nullptr;
  }
}

//funktion used in recive data callback funktion to register unregisted peers
void RegisterPeers(struct SensordataTp MacToAdd) {
  int tempIndex;
  uint8_t EMPTY_MAC_ADDRESS[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  // loop until u find a spot in the array thats empty
  for (int j = 0; j < 10; j++) {
    if (memcmp(&Peers[j].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(Peers[j].peerInfo.peer_addr)) == 0) {
      tempIndex = j;
      break;
    } else {
      continue;
    }
  }
  //then overwrite that empty spot in the array with the mac-address to add
  memcpy(&Peers[tempIndex].peerInfo.peer_addr, MacToAdd.id, sizeof(Peers[tempIndex].peerInfo.peer_addr));

  Peers[tempIndex].peerInfo.channel = 0;
  Peers[tempIndex].peerInfo.encrypt = false;

  Serial.println("Peer MAC:");
  AddressOfPeer(Peers[tempIndex].peerInfo.peer_addr);

  //code for checking if the connection was a success, if it was then it send the confirmation back the slave
  if (esp_now_add_peer(&Peers[tempIndex].peerInfo) == ESP_OK) {
    Serial.println("Peer added successfully");
    Peers[tempIndex].isActive = true;
    CommandStruct.command = 'C';  //command for connection confirmed
    esp_now_send(Peers[tempIndex].peerInfo.peer_addr, (uint8_t*)&CommandStruct, sizeof(CommandStruct));
  } else {
    Serial.print("Failed to add peer Error code: ");
    Serial.println(esp_now_add_peer(&Peers[tempIndex].peerInfo));
  }
}

//format the mac-address into hexcode (the standard way of displaying a mac-address)
void AddressOfPeer(uint8_t peerInfoMAC[]) {
  for (int i = 0; i < 6; i++) {
    if (peerInfoMAC[i] < 16) { Serial.print("0"); }  // zero pad 
    Serial.print(peerInfoMAC[i], HEX);
    if (i < 5) { Serial.print(":"); }
  }
}

//draws the screen using library funktions based on the VT100 standard thing
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
  tft.print("Humid: ");
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
  CommandStruct.command = command;  //command for sending the data confirmed

  for (int i = 0; i < 10; i++) {
    esp_now_send(Peers[i].peerInfo.peer_addr, (uint8_t*)&CommandStruct, sizeof(TempIngoingStruct));
  }
}

//calculates the average by using the varriables stored in Sensordata stuct (likely the AveragesStruct)
void CalculateAverage(struct SensordataTp* resultStruct) {
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
    }
  }
  resultStruct->temp /= activePeersTotal;
  resultStruct->hum /= activePeersTotal;
  resultStruct->co2 /= activePeersTotal;
  resultStruct->activePeersTotal = activePeersTotal;
  Serial.println(activePeersTotal);
}

//sets a time on every incomeing sensordata peer to keep track of when that peer was last seen,
// if it not been seen in a while then remove it, set its mac-adr's spot in array to 0, and restart the SLAVE 
void PruneUnresponsivePeers() {
  unsigned long currentTime = millis();
  const unsigned long TIMEOUT_MS = refreshInterval+500;  //3 seconds timeout

  for (int i = 0; i < 10; i++) {
    if (Peers[i].isActive == true) {
      if (currentTime - Peers[i].lastSeenTime > TIMEOUT_MS) {
        // Peer has timed out.
        Serial.println("Peer timed out. Sending Reset command and removing.");

        CommandStruct.command = 'R';
        esp_now_send(Peers[i].peerInfo.peer_addr, (uint8_t*)&CommandStruct, sizeof(CommandStruct));
        CommandStruct.command = ' ';

        //Remove the peer using the ESP-NOW library function
        esp_now_del_peer(Peers[i].peerInfo.peer_addr);
        Peers[i].isActive = false;

        //clear address
        uint8_t EMPTY_MAC_ADDRESS[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        memcpy(&Peers[i].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(EMPTY_MAC_ADDRESS));
      }
    }
  }
}
