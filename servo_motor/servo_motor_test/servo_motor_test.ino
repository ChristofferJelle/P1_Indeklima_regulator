#include <Servo.h> //library for servo

const int servoPin1 = 9;
const int servoPin2 = 8;
const int servoPin3 = 7;
Servo servo1; //create servo object
Servo servo2;
Servo servo3;

void setup() {
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
}

void loop() {
  servo1.write(0);
  servo2.write(0);
  servo3.write(0);
  delay(1000);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  delay(1000);
  servo1.write(180);
  servo2.write(180);
  servo3.write(180);
  delay(1000);
}