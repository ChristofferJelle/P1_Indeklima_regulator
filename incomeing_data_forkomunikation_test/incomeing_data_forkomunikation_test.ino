#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

struct Sensordata {
  float temp;
  float hum;
  float pres;
};
// Create a struct to hold sensor readings
Sensordata IngoingStruct;

// Define variables to store incoming readings
float incomingTemp;
float incomingHum;
float incomingPres;


uint8_t* readMacAddress() {
  static uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_AP, baseMac);
  if (ret == ESP_OK) {
    return baseMac;
  } else {
    Serial.println("Failed to read MAC address");
    return nullptr;
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&IngoingStruct, incomingData, sizeof(IngoingStruct));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTemp = IngoingStruct.temp;
  incomingHum = IngoingStruct.hum;
  incomingPres = IngoingStruct.pres;
}

void setup() {
  Serial.begin(115200);

  //initialises what kinda wifi mode its in, rn its in accese point mode, so i dont have to connect to uni wifi with own student password.
  WiFi.mode(WIFI_AP);
  WiFi.AP.begin();
  //har
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  uint8_t* ownMac = readMacAddress();
  //converts array into a string.
  if (ownMac != nullptr) {
    String ownMacHex;
    for (int i = 0; i < 6; i++) {
      String hexPart = String(ownMac[i], HEX);
      if (hexPart.length() < 2) hexPart = "0" + hexPart;  // zero-pad if needed
      ownMacHex += hexPart;
      if (i < 5) ownMacHex += ":";  // add ':' except after last byte
    }
    Serial.print(ownMacHex);
  }

  //initatite ESP_now for comunication without router.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Register that i want to send to cb. basically says "everytime i send data u just need to run this too"
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataRecv));

  // Register peers

}

void loop() {
}