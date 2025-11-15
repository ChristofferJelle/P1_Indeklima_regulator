#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>
//make float array with all data in it, maybe #define temp array[3] = temp

struct Sensordata {
  float temp = 200;
  float hum = 2000;
  float co2 = 24000;
  //int id = 1;
};
// Create a struct to hold sensor readings
Sensordata outgoingStruct;

//adress of the mainESP32 in sending data to
uint8_t broadcastAddress[] = { 0x08, 0x3a, 0xf2, 0x45, 0x3f, 0x50 };

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  //initatite ESP_now for comunication without router.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Register that i want to send to cb. basically says "everytime i send data u just need to run this too"
  //esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));

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

void loop() {

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&outgoingStruct, sizeof(outgoingStruct));

  if (result == ESP_OK) {
    Serial.print("Sent with success to ");
    AddressOfPeer(broadcastAddress);
    Serial.println();
  } else {
    Serial.print("Error sending the data to ");

    AddressOfPeer(broadcastAddress);
    Serial.println();
  }
}