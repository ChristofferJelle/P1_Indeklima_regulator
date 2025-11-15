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

  InitESP32_NOW();
  registerPeers();
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