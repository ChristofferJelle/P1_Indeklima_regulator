#include <Servo.h> //library for servo

const int servoPin = 9;
Servo servo; //create servo object

void setup() {
  servo.attach(servoPin);
}

void loop() {
  servo.write(0);
  delay(1000);
  servo.write(90);
  delay(1000);
  servo.write(180);
  delay(1000);
}