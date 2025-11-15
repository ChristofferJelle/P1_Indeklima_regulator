
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

void AddressOfPeer(uint8_t peerInfoMAC[]) {
  for (int i = 0; i < 6; i++) {
    if (peerInfoMAC[i] < 16) { Serial.print("0"); }  // zero pad
    Serial.print(peerInfoMAC[i], HEX);
    if (i < 5) { Serial.print(":"); }
  }
}