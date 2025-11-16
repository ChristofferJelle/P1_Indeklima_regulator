
/*
//parameter automatically get fillede when callback funktion is called.
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  Serial.print("Send to: ");

  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" -> ");
  Serial.println(ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
}
*/

void InitESP32_NOW() {
  WiFi.mode(WIFI_STA);

  //initatite ESP_now for comunication without router.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Register that i want to send to cb. basically says "everytime i send data u just need to run this too"
  //esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  //Register that i want to recive to cb. basically says "everytime i send data u just need to run this too"
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&outgoingStruct, incomingData, sizeof(outgoingStruct));
  Serial.println(CommandStruct.command);
  switch (CommandStruct.command) {
    case 'C':
      ConnectedToMaster = true;
      break;
    case 'S':
      outgoingStruct.ping = '2';
      SendDataToMaster();
      break;
    case 'R':
      ConnectedToMaster = false;
      ESP.restart();
      break;
    default:
      break;
  }
}
void SendDataToMaster() {
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&outgoingStruct, sizeof(outgoingStruct));

  /*
  if (result == ESP_OK) {
    Serial.print("Sent with success to ");
    AddressOfPeer(broadcastAddress);
    Serial.println();
  } else {
    Serial.print("Error sending the data to ");

    AddressOfPeer(broadcastAddress);
    Serial.println();
  }
  */
}


void registerPeers() {

  // Register peer
  for (int i = 0; i < 6; i++) {
    peerInfo.peer_addr[i] = broadcastAddress[i];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  Serial.println("Peer MAC:");
  AddressOfPeer(broadcastAddress);

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("Peer added successfully");
  } else {
    Serial.print("Failed to add peer Error code: ");
    Serial.println(esp_now_add_peer(&peerInfo));
  }
}

void AddressOfPeer(uint8_t peerInfoMAC[]) {
  for (int i = 0; i < 6; i++) {
    if (peerInfoMAC[i] < 16) { Serial.print("0"); }  // zero pad
    Serial.print(peerInfoMAC[i], HEX);
    if (i < 5) { Serial.print(":"); }
  }
}