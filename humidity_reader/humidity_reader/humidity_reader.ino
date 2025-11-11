#include "DHT.h"

const int DHTPin = 2;
DHT dht(DHTPin, DHT11);

void setup() {
  Serial.begin(115200);
  dht.begin(); //initialise DHT sensor
}

void loop() {
  delay(2000);
  dhtRead();
}

void dhtRead() {
  //reads can take up to 250ms
  float humidity = dht.readHumidity();

  if (isnan(humidity)) { //check if any reads failed and exit early
    Serial.println("Failed to read humidity.");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%");
}
