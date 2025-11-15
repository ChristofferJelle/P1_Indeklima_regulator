#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <TFT_eSPI.h>  // LILYGO T-Display library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Create TFT object

struct Sensordata {
  float temp;
  float hum;
  float co2;
  //int id;
};
// Create a struct to hold sensor readings
Sensordata IngoingStruct;

int refreshTimer = 10000;
int timerReset = refreshTimer + millis();

void setup() {
  Serial.begin(115200);

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

  // Register peers
  
}

void loop() {
  if (millis() >= timerReset) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.println("INCOMING READINGS");

    tft.setCursor(0, 30);
    tft.print("Temp: ");
    tft.print(IngoingStruct.temp, 1);  // 1 decimal place
    tft.println(" C");

    tft.setCursor(0, 60);
    tft.print("Hum: ");
    tft.print(IngoingStruct.hum, 1);
    tft.println(" %");

    tft.setCursor(0, 90);
    tft.print("co2: ");
    tft.print(IngoingStruct.co2, 1);
    tft.println(" ppm");

    // Serial output remains the same
    Serial.println("INCOMING READINGS");
    Serial.print("Temperature: ");
    Serial.print(IngoingStruct.temp);
    Serial.println(" ºC");
    Serial.print("Humidity: ");
    Serial.print(IngoingStruct.hum);
    Serial.println(" %");
    Serial.print("Pressure: ");
    Serial.print(IngoingStruct.co2);
    Serial.println(" ppm");
    Serial.println();
    timerReset += refreshTimer;
  }
}