#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <TFT_eSPI.h>  // LILYGO T-Display library
#include <SPI.h>

#define BUTTON_PIN 35

TFT_eSPI tft = TFT_eSPI();  // Create TFT object

esp_now_peer_info_t peerInfo[10];

struct Sensordata {
  float temp;
  float hum;
  float co2;
  uint8_t id[6];
  char command;
};

// Create a struct to hold sensor readings
Sensordata IngoingStruct, TempIngoingStruct, CommandStruct;

int refreshTimer = 10000;
int timerReset = refreshTimer + millis();

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  Serial.println();

  InitDisplay();

  InitESP32_NOW();

  //i could also just do Serial.println(WiFi.macAddress()); i guess
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
    Serial.println(ownMacHex);
  }
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {
    Serial.println("Button pressed!");
    SendCommandAllSlaves('R');
    ESP.restart();
  }

  if (millis() >= timerReset) {
    SendCommandAllSlaves('S');

    DrawDisplay();
    timerReset += refreshTimer;
  }
}