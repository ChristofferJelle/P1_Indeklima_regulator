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

uint8_t* readMacAddress() {
  static uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    return baseMac;
  } else {
    Serial.println("Failed to read MAC address");
    return nullptr;
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&IngoingStruct, incomingData, sizeof(IngoingStruct));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);  // landscape mode
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);  // scale text


  //initialises what kinda wifi mode its in, rn its in accese point mode, so i dont have to connect to uni wifi with own student password.
  WiFi.mode(WIFI_STA);
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
    Serial.print(ownMacHex);
  }

  //initatite ESP_now for comunication without router.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }


  //Register that i want to recive to cb. basically says "everytime i send data u just need to run this too"
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
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