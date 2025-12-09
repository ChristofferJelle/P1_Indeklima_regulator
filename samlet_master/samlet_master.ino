//ESP:
//remember to edit library header files https://jensd.dk/doc/esp32/esp32s3.html
#include <TFT_eSPI.h> //LILYGO T-Display library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI(); //create TFT object

#define BUTTON_PIN 35

//------------------------------------------------------------------------------
//wireless communication:
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

struct SensordataTp {
  float temp;
  float humid;
  float co2;
  uint8_t id[6]; //uint8_t because only the last 2 digits are used
  char command;
  int activePeers;
};
struct SensordataTp commandStruct, averagesStruct;

struct PeerDataTp {
  esp_now_peer_info_t peerInfo;
  struct SensordataTp incomingStruct;
  bool isActive;
  unsigned long lastSeenTime;
};
const int maxPeers = 10;
struct PeerDataTp peersArr[maxPeers];

unsigned long lastRefresh = 0;
const unsigned long dataRequestInterval = 4000;

//------------------------------------------------------------------------------
//servo:
#include <ESP32Servo.h> //library for servo
#define SERVO_PIN 17

Servo servo; //create servo object

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

//------------------------------------------------------------------------------
//rotary encoder:
#define CLK_PIN 37 //1st click
#define DT_PIN 38 //2nd click
#define SW_PIN 39 //button click

struct SensorDataLimitTp {
  long temp = 25;
  long humid = 60;
  long co2 = 800;
  char currentSensorData = 'T'; //can be 'T', 'H', or 'C'
};
struct SensorDataLimitTp upperLimits;

enum RotaryEncoderStateTp {
  idleState,
  timeout
};
RotaryEncoderStateTp rotaryEncoderState = idleState;

int currentStateCLK;
int lastStateCLK;
unsigned long rotaryLastRefresh = 0;
unsigned long limitDisplayLastRefresh = 0;
const unsigned long limitDisplayRefreshInterval = 100;
const unsigned long rotaryRefreshInterval = 1100;

int prevButtonState;
unsigned int ButtonPresses = 0;

void setup() {
  Serial.begin(115200);

  InitESP();
  InitRotaryEncoder();
  InitServo();

  //could also just do Serial.println(WiFi.macAddress());, but this is cooler
  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");
  uint8_t* ownMac = GetOwnMacAddress();
  //convert array into a string
  if (ownMac != nullptr) {
    String ownMacHex;
    for (int i = 0; i < 6; i++) {
      String hexPart = String(ownMac[i], HEX);
      if (hexPart.length() < 2) hexPart = "0" + hexPart; //zero-pad if needed
      ownMacHex += hexPart;
      if (i < 5) {ownMacHex += ":";} //add ':' except after last byte
    }
    Serial.println(ownMacHex);
  }
}

void loop() {
  ReadEncoder();

  unsigned long timeNow = millis();
  if(timeNow - rotaryLastRefresh >= rotaryRefreshInterval) {
    rotaryEncoderState = idleState;
  } else if(rotaryEncoderState == timeout
            && timeNow - limitDisplayLastRefresh >= limitDisplayRefreshInterval) {
    DrawLimitValues();
    limitDisplayLastRefresh = timeNow;
  }

  if(rotaryEncoderState != timeout) {
    int buttonState = digitalRead(BUTTON_PIN);
    if(!buttonState) {
      Serial.println("Button pressed!");
      SendCommandAllSlaves('R');
      ESP.restart();
    }

    float shuntVoltage = ShuntVoltage();

    if(millis() - dataRequestInterval > lastRefresh) {
      PruneUnresponsivePeers();
      SendCommandAllSlaves('S');
      CalculateAverage(&averagesStruct);
      DrawDisplay();
      lastRefresh = millis();
    }

    if(!shuntTimeout) {
      if((averagesStruct.temp >= upperLimits.temp)
      || (averagesStruct.humid >= upperLimits.humid)
      || (averagesStruct.co2 >= upperLimits.co2)) {
        ServoOpen();
      } else {
        ServoClose();
      }
    }

    if(shuntVoltage > 2.94 && !shuntTimeout) {
      shuntTimeout = true;
    }
    if(shuntTimeout && !shuntActionDone) {
      if(servoState == sweepOpen) {
        ServoClose();
      } else if(servoState == sweepClose) {
        ServoOpen();
      }
      lastShuntTime = millis();
      shuntActionDone = true;
      servoState = idle;
    }

    //reset timeout when time runs out
    if((millis() - lastShuntTime >= shuntInterval) && shuntTimeout && shuntActionDone) {
      shuntTimeout = false;
      shuntActionDone = false;
    }
  }
}
