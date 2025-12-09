//callback function for when data is received
void OnDataReceived(const uint8_t* mac, const uint8_t* incomingData) {
  struct SensordataTp temporaryIncomingStruct;

  memcpy(&temporaryIncomingStruct, incomingData, sizeof(temporaryIncomingStruct));

  if(CheckPeerId(temporaryIncomingStruct.id) >= 0) { //if peer is already registered
    for(int i = 0; i < maxPeers; i++) {
      int indexNumber = CheckPeerId(temporaryIncomingStruct.id);

      memcpy(&peersArr[indexNumber].incomingStruct, incomingData, sizeof(peersArr[indexNumber].incomingStruct));
      peersArr[indexNumber].lastSeenTime = millis(); //update peer's last seen time
      break;
    }
  } else {
    Serial.println("New peer detected. Adds to list of peers.");
    RegisterPeer(temporaryIncomingStruct.id);
  }
}

//check whether any registered peer has the incoming MAC-address
int CheckPeerId(uint8_t MacToCheck[6]) {
  for (int i = 0; i < 10; i++) {
    if (memcmp(MacToCheck, peersArr[i].peerInfo.peer_addr, sizeof(peersArr[i].peerInfo.peer_addr)) == 0) {
      return i;
    }
  }
  return -1;
}

//used in OnDataRecieved() function to register unregistered peers
void RegisterPeer(uint8_t newMac[6]) {
  const uint8_t EMPTY_MAC_ADDRESS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //uint8_t because only the last 2 digits are used

  //loop until empty spot in array is found
  for (int i = 0; i < maxPeers; i++) {
    if (memcmp(&peersArr[i].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(peersArr[i].peerInfo.peer_addr)) == 0) { //check whether the 2 are equal
      //overwrite empty spot in peersArr with the new MAC-address
      memcpy(&peersArr[i].peerInfo.peer_addr, newMac, sizeof(peersArr[i].peerInfo.peer_addr));

      peersArr[i].peerInfo.channel = 0; //use same Wi-Fi channel as station (aka main)
      peersArr[i].peerInfo.encrypt = false; //no encryption

      Serial.println("Peer MAC:");
      PrintAddressOfPeer(peersArr[i].peerInfo.peer_addr);

      //check whether connection was a success
      if(esp_now_add_peer(&peersArr[i].peerInfo) == ESP_OK) {
        Serial.println("Peer added successfully.");
        peersArr[i].isActive = true;
        commandStruct.command = 'C'; //command for connection confirmed
        esp_now_send(peersArr[i].peerInfo.peer_addr, (uint8_t*)&commandStruct, sizeof(commandStruct)); //send confirmation to slave
      } else {
        Serial.print("Failed to add peer. Error code: ");
        Serial.println(esp_now_add_peer(&peersArr[i].peerInfo));
      }
    }
  }
}

//format the MAC-address into hexcode (the standard way of displaying MAC-addresses)
void PrintAddressOfPeer(uint8_t peerMac[]) {
  for (int i = 0; i < 6; i++) {
    if (peerMac[i] < 16) {Serial.print("0");} //zero pad 
    Serial.print(peerMac[i], HEX);
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
  tft.print("Temperature: ");
  tft.print(averagesStruct.temp, 1); //1 decimal place
  tft.print(" "); tft.write(0xF7); tft.println("C");

  tft.setCursor(0, 60);
  tft.print("Humidity: ");
  tft.print(averagesStruct.humid);
  tft.println("%");

  tft.setCursor(0, 90);
  tft.print("CO2: ");
  tft.print(427 + averagesStruct.co2); //427 is for calibration
  tft.println(" ppm");

  tft.setCursor(0, 120);
  tft.print("Connected Peers: ");
  tft.print(averagesStruct.activePeers);
}

void SendCommandAllSlaves(char command) {
  commandStruct.command = command;

  for (int i = 0; i < maxPeers; i++) {
    esp_now_send(peersArr[i].peerInfo.peer_addr, (uint8_t*)&commandStruct, sizeof(commandStruct));
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
      resultStruct->temp += peersArr[i].incomingStruct.temp;
      resultStruct->humid += peersArr[i].incomingStruct.humid;
      resultStruct->co2 += peersArr[i].incomingStruct.co2;
      activePeersTotal++; 
    }
  }
  resultStruct->temp /= activePeersTotal;
  resultStruct->humid /= activePeersTotal;
  resultStruct->co2 /= activePeersTotal;
  resultStruct->activePeers = activePeersTotal;
}

//if slave has not been seen in a while, remove it, set its mac-adsress spot in peersArr to 0, and restart the peer (slave)
void PruneUnresponsivePeers() {
  unsigned long currentTime = millis();
  const unsigned long TIMEOUT_MS = dataRequestInterval + 2000; //peers has 2 seconds to respond before being pruned

  for (int i = 0; i < maxPeers; i++) {
    if (peersArr[i].isActive
    && (currentTime - TIMEOUT_MS > peersArr[i].lastSeenTime)) { //if peer has timed out
      Serial.println("Peer timed out. Sending reset-command and removing.");

      commandStruct.command = 'R';
      esp_now_send(peersArr[i].peerInfo.peer_addr, (uint8_t*)&commandStruct, sizeof(commandStruct));
      commandStruct.command = ' '; //reset command after sending

      //remove the peer using the ESP-NOW library function
      esp_now_del_peer(peersArr[i].peerInfo.peer_addr);
      peersArr[i].isActive = false;

      //clear address
      const uint8_t EMPTY_MAC_ADDRESS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //uint8_t because only the last 2 digits are used
      memcpy(&peersArr[i].peerInfo.peer_addr, EMPTY_MAC_ADDRESS, sizeof(peersArr[i].peerInfo.peer_addr));
    }
  }
}
