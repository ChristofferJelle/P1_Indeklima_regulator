// Callback when data is received
void OnDataReceived(const uint8_t* mac, const uint8_t* incomingData) {
  memcpy(&TemporaryIngoingStruct, incomingData, sizeof(TemporaryIngoingStruct));
  // The condition reads: "If the first 6 bytes of peerInfo are NOT equal to the first 6 bytes of IngoingStruct.id"
  if (CheckArrayList(TemporaryIngoingStruct) >= 0) {
    //Serial.println("Peer already registered");

    for (int i = 0; i < 10; i++) {
      int IDIndexMatch = CheckArrayList(TemporaryIngoingStruct);
      if (IDIndexMatch >= 0) {
        memcpy(&peersArr[IDIndexMatch].IngoingStruct, incomingData, sizeof(peersArr[IDIndexMatch].IngoingStruct));
        // Update peer last seen time
        peersArr[IDIndexMatch].lastSeenTime = millis();
        break;
      }
    }
  } else {
    Serial.println("who tf are you, get in here");
    RegisterPeers(TemporaryIngoingStruct);
  }
}

// funktion to check arraylist if any of its arrays contain the incomeing macaddress
int CheckArrayList(struct SensordataTp MacToCheck) {
  for (int i = 0; i < 10; i++) {
    if (memcmp(MacToCheck.id, peersArr[i].peerInfo.peer_addr, sizeof(peersArr[i].peerInfo.peer_addr)) == 0) {
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
  const uint8_t EMPTY_MAC_ADDRESS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //uint8_t because only the last 2 digits are used
  // loop until u find a spot in the array thats empty
  for (int i = 0; i < 10; i++) {
    if (memcmp(&peersArr[i].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(peersArr[i].peerInfo.peer_addr)) == 0) {
      tempIndex = i;
      break;
    } else {
      continue;
    }
  }
  //then overwrite that empty spot in the array with the mac-address to add
  memcpy(&peersArr[tempIndex].peerInfo.peer_addr, MacToAdd.id, sizeof(peersArr[tempIndex].peerInfo.peer_addr));

  peersArr[tempIndex].peerInfo.channel = 0;
  peersArr[tempIndex].peerInfo.encrypt = false;

  Serial.println("Peer MAC:");
  PrintAddressOfPeer(peersArr[tempIndex].peerInfo.peer_addr);

  //code for checking if the connection was a success, if it was then it send the confirmation back the slave
  if (esp_now_add_peer(&peersArr[tempIndex].peerInfo) == ESP_OK) {
    Serial.println("Peer added successfully");
    peersArr[tempIndex].isActive = true;
    CommandStruct.command = 'C';  //command for connection confirmed
    esp_now_send(peersArr[tempIndex].peerInfo.peer_addr, (uint8_t*)&CommandStruct, sizeof(CommandStruct));
  } else {
    Serial.print("Failed to add peer Error code: ");
    Serial.println(esp_now_add_peer(&peersArr[tempIndex].peerInfo));
  }
}

//format the mac-address into hexcode (the standard way of displaying mac-addresses)
void PrintAddressOfPeer(uint8_t peerInfoMAC[]) {
  for (int i = 0; i < 6; i++) {
    if (peerInfoMAC[i] < 16) {Serial.print("0");} //zero pad 
    Serial.print(peerInfoMAC[i], HEX);
    if (i < 5) {Serial.print(":");} //print ':' between each number in address
  }
}

//draws the screen using library functions based on the VT100 standard
void DrawDisplay() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("INCOMING READINGS");

  tft.setCursor(0, 30);
  tft.print("Temp: ");
  tft.print(AveragesStruct.temp, 1); //1 decimal place
  tft.println(" C");

  tft.setCursor(0, 60);
  tft.print("Humid: ");
  tft.print(AveragesStruct.humid);
  tft.println(" %");

  tft.setCursor(0, 90);
  tft.print("CO2: ");
  tft.print(400 + AveragesStruct.co2);
  tft.println(" ppm");

  tft.setCursor(0, 120);
  tft.print("Connected Peers: ");
  tft.print(AveragesStruct.activePeers);

  //serial output remains the same
  Serial.println("INCOMING READINGS");
  Serial.print("Temperature: ");
  Serial.print(AveragesStruct.temp);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(AveragesStruct.humid);
  Serial.println(" %");
  Serial.print("Pressure: ");
  Serial.print(400 + AveragesStruct.co2);
  Serial.println(" ppm");
  Serial.println();
}

void SendCommandAllSlaves(char command) {
  CommandStruct.command = command; //command for sending the data confirmed

  for (int i = 0; i < 10; i++) {
    esp_now_send(peersArr[i].peerInfo.peer_addr, (uint8_t*)&CommandStruct, sizeof(TemporaryIngoingStruct));
  }
}

//calculates average of the values recieved from each peer
void CalculateAverage(struct SensordataTp* resultStruct) {
  //reset values of resultStruct
  resultStruct->temp = 0;
  resultStruct->humid = 0;
  resultStruct->co2 = 0;
  resultStruct->activePeers = 0;

  int activePeersTotal = 0;
  for(int i = 0; i < maxPeers; i++) {
    if(peersArr[i].isActive) {
      resultStruct->temp += peersArr[i].IngoingStruct.temp;
      resultStruct->humid += peersArr[i].IngoingStruct.humid;
      resultStruct->co2 += peersArr[i].IngoingStruct.co2;
      activePeersTotal++; 
    }
  }
  resultStruct->temp /= activePeersTotal;
  resultStruct->humid /= activePeersTotal;
  resultStruct->co2 /= activePeersTotal;
  resultStruct->activePeers = activePeersTotal;
}

//if slave has not been seen in a while, remove it, set its mac-adress spot in peersArr to 0, and restart the peer (slave)
void PruneUnresponsivePeers() {
  unsigned long currentTime = millis();
  const unsigned long TIMEOUT_MS = dataRequestInterval + 2000; //peers has 2 seconds to respond before being pruned

  for (int i = 0; i < maxPeers; i++) {
    if (peersArr[i].isActive
    && (currentTime - TIMEOUT_MS > peersArr[i].lastSeenTime)) { //if peer has timed out
      Serial.println("Peer timed out. Sending reset-command and removing.");

      CommandStruct.command = 'R';
      esp_now_send(peersArr[i].peerInfo.peer_addr, (uint8_t*)&CommandStruct, sizeof(CommandStruct));
      CommandStruct.command = ' '; //reset command after sending

      //remove the peer using the ESP-NOW library function
      esp_now_del_peer(peersArr[i].peerInfo.peer_addr);
      peersArr[i].isActive = false;

      //clear address
      const uint8_t EMPTY_MAC_ADDRESS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //uint8_t because only the last 2 digits are used
      memcpy(&peersArr[i].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(EMPTY_MAC_ADDRESS));
    }
  }
}
