//ESP:
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

//remember to edit library header files https://jensd.dk/doc/esp32/esp32s3.html
#include <TFT_eSPI.h> // LILYGO T-Display library
#include <SPI.h>

//servo
#include <ESP32Servo.h> //library for servo
const int servoPin = 17;

Servo servo;  //create servo object
enum ServoStateTp {
  sweepOpen,
  sweepClose,
  idle //triggered by shunt hit
};
ServoStateTp servoState = sweepClose;
#define SHUNT_PIN 32
unsigned long lastShuntTime = 0;
const unsigned long shuntInterval = 1000;
bool shuntTimeout = false;
bool shuntActionDone = false;

TFT_eSPI tft = TFT_eSPI();  // Create TFT object
#define BUTTON_PIN 35

//Rotary encoder:
#define CLK_PIN 37 //1st click
#define DT_PIN 38 //2nd click
#define SW_PIN 39 //button click
bool interrupt = false;

struct SensorDataLimitTp {
  long Temp = 30;
  long Humid = 50;
  long CO2 = 600;
  char CurrentSensorData = 'T';
};
struct SensorDataLimitTp s1;

enum RotaryEncoderStateTp {
  idle,
  timeout
};
RotaryEncoderStateTp rotaryEncoderState = idle;
int currentStateCLK;
int lastStateCLK;
unsigned long rotaryLastRefresh = 0;
unsigned long limitDisplayLastRefresh = 0;
const unsigned long limitDisplayRefreshInterval = 100;
const unsigned long rotaryRefreshInterval = 1100;

int prevButtonSate;
unsigned int ButtonPresses = 0;

//hack
struct SensordataTp {
  float temp;
  float hum;
  float co2;
  uint8_t id[6];
  char command;
  int activePeersTotal;
};
// Create a struct to hold sensor readings
SensordataTp TempIngoingStruct, CommandStruct, AveragesStruct;

struct PeerDataContextTp {
  esp_now_peer_info_t peerInfo;
  struct SensordataTp IngoingStruct;
  bool isActive; //Flag to track active peers
  unsigned long lastSeenTime;
};
PeerDataContextTp Peers[10];
unsigned long lastRefresh = 0;
const unsigned long refreshInterval = 4000;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  pinMode(SHUNT_PIN, INPUT);
  Serial.println();

  InitDisplay();

  InitESP32_NOW();

  //i could also just do Serial.println(WiFi.macAddress()); i guess, but this is cooler
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  uint8_t* ownMac = ReadMacAddress();
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
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  attachInterrupt(CLK_PIN, InterruptCallback, FALLING);

  prevButtonSate = digitalRead(SW_PIN);

  Serial.println(prevButtonSate);
  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK_PIN);
  currentStateCLK = digitalRead(CLK_PIN);
  //servo:
  servo.attach(servoPin);
  ServoClose();
}

void loop() {
  ReadEncoder();
  unsigned long timeNow = millis();
  if (timeNow - rotaryLastRefresh >= rotaryRefreshInterval) {
    rotaryEncoderState = idle;
  } else if (rotaryEncoderState == timeout && timeNow - limitDisplayLastRefresh >= limitDisplayRefreshInterval) {
    DrawLimitValues();
    limitDisplayLastRefresh = timeNow;
  }

  if (rotaryEncoderState != timeout) {
    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW) {
      Serial.println("Button pressed!");
      SendCommandAllSlaves('R');
      ESP.restart();
    }

    float shuntCurrent = ShuntCurrent();

    if (millis() - lastRefresh >= refreshInterval) {
      PruneUnresponsivePeers();
      SendCommandAllSlaves('S');
      CalculateAverage(&AveragesStruct);
      DrawDisplay();
      lastRefresh = millis();
    }

    if (!shuntTimeout) {
      if ((AveragesStruct.temp >= s1.Temp || AveragesStruct.hum >= s1.Humid) || AveragesStruct.co2 >= s1.CO2) {
        ServoOpen();
      } else {
        ServoClose();
      }
    }

    if (shuntCurrent > 2.94 && !shuntTimeout) {

      shuntTimeout = true;
    }
    // Vi er i timeout-tilstand → skriv servo-position
    if (shuntTimeout && !shuntActionDone) {
      if (servoState == sweepOpen) {
        servo.write(0);
      } else if (servoState == sweepClose) {
        servo.write(180);
      }
      lastShuntTime = millis();
      shuntActionDone = true;
      servoState = idle;
    }

    // Reset timeout når tiden er gået
    if ((millis() - lastShuntTime >= shuntInterval && shuntTimeout) && shuntActionDone) {
      shuntTimeout = false;
      shuntActionDone = false;
    }
  }
}