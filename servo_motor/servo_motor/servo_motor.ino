#include <Servo.h> //library for servo

const int servoPin = 9;
Servo servo; //create servo object

void setup() {
  servo1.attach(servoPin);
}

void loop() {
  ServoClose();
  delay(1000);
  ServoOpen();
  delay(1000);
}

void ServoClose() {
  servo.write(0);
}

void ServoOpen() {
  servo.write(180);
}