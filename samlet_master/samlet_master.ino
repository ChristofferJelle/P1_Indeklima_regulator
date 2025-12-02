//ESP:
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

//remember to edit library header files https://jensd.dk/doc/esp32/esp32s3.html
#include <TFT_eSPI.h>  // LILYGO T-Display library
#include <SPI.h>

//serva
#include <ESP32Servo.h>  //library for servo
const int servoPin = 17;
const int shuntPin = 36;
Servo servo;  //create servo object

TFT_eSPI tft = TFT_eSPI();  // Create TFT object
#define BUTTON_PIN 35

//Rotary encoder:
#define CLK 37  //1st click
#define DT 38   //2nd click
#define SW 39   //button click

struct SensorData {
  int Temp = 20;
  int Humid = 50;
  int CO2 = 30;
  char CurrentSensorData = 'T';
};

struct SensorData s1;

int currentStateCLK;
int lastStateCLK;

int prevButtonSate;
unsigned int ButtonPresses = 0;



struct Sensordata {
  float temp;
  float hum;
  float co2;
  uint8_t id[6];
  char command;
  int activePeersTotal;
};
// Create a struct to hold sensor readings
Sensordata TempIngoingStruct, CommandStruct, AveragesStruct;

struct PeerDataContext {
  esp_now_peer_info_t peerInfo;
  Sensordata IngoingStruct;
  bool isActive;               // Flag to track active peers
  unsigned long lastSeenTime;  // <--- Add this
};
PeerDataContext Peers[10];
int refreshTimer = 2000;
int timerReset = refreshTimer + millis();



void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  Serial.println();

  InitDisplay();

  InitESP32_NOW();

  //i could also just do Serial.println(WiFi.macAddress()); i guess, but this is cooler
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

  //rotary encoder:
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  prevButtonSate = digitalRead(SW);

  Serial.println(prevButtonSate);
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);

  //servo:
  servo.attach(servoPin);
  ServoClose();
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {
    Serial.println("Button pressed!");
    SendCommandAllSlaves('R');
    ESP.restart();
  }
  if (ShuntCurrent() < 0.2) {
    delay(1000);
    ServoClose();
    if (millis() >= timerReset) {
      PruneUnresponsivePeers();
      SendCommandAllSlaves('S');
      CalculateAvrg(&AveragesStruct);
      DrawDisplay();
      timerReset += refreshTimer;
      ServoOpen();
    }
  } else {
    servo.release();
    delay(1500);
  }

  readEncoder();
}