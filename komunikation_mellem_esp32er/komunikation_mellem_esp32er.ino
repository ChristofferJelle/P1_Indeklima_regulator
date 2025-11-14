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




/*
esp_wifi_get_mac: attempts to Get mac of specified interface (ifx) in its own acces point
and store mac of the interface ifx in an array.
then returns if it succesded or not in getting the mac

then with printf i specify it the format it needs to spit out the mac address in. %02x = print as hexidecimal number, that needs to be at least 2 digits wide.
*/
esp_now_peer_info_t peerInfo;

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

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  
  //initialises what kinda wifi mode its in, rn its in STA mode, so i dont have to connect to uni wifi with own student password.


  //initatite ESP_now for comunication without router.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //Register that i want to send to cb. basically says "everytime i send data u just need to run this too"
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

/*
  Serial.print("Peer MAC: ");
  for (int i = 0; i < 6; i++) {
    if (peerInfo.peer_addr[i] < 16) Serial.print("0");  // zero pad
    Serial.print(peerInfo.peer_addr[i], HEX);
    if (i < 5) Serial.print(":");
  }

  Serial.println();
*/

  if (esp_now_add_peer(&peerInfo) == ESP_OK) {
    Serial.println("Peer added successfully");
  } else {
    Serial.print("Failed to add peer Error code: ");
    Serial.println(esp_now_add_peer(&peerInfo));
  }
  
}



void loop() {

   // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &outgoingStruct, sizeof(outgoingStruct));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  
   
}