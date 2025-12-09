#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>
//make float array with all data in it, maybe #define temp array[3] = temp

struct Sensordata {
  float temp = 120;
  float hum = 20000;
  float co2 = 24000;
  uint8_t id[6];
  char command;
  int activePeersTotal;
};
// Create a struct to hold sensor readings
Sensordata outgoingStruct, CommandStruct;

//adress of the mainESP32 in sending data to
uint8_t broadcastAddress[] = { 0x08, 0x3a, 0xf2, 0x45, 0x3f, 0x50 };

esp_now_peer_info_t peerInfo;

bool ConnectedToMaster = false;


void setup() {
  Serial.begin(115200);
  InitESP32_NOW();

  esp_wifi_get_mac(WIFI_IF_STA, outgoingStruct.id);

  registerPeers();
  //outgoingStruct.id = WiFi.macAddress();
}

//if its not connected to the master/main esp32 then keep trying to ping with it's outgoing struct, which includes its own mac address as an id so it can be added as a peer, aka be connected
//it gets conncted when the master returns the 'R' command, which sets ConnectedToMaster to false in the OnDataRecv() callback funktion
void loop() {
  if (ConnectedToMaster == false) {
    SendDataToMaster();
  }
}