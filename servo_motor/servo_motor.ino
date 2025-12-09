#include <ESP32Servo.h>

const int servoPin = 13;
Servo servo; //create servo object

void setup () {
  servo.attach(servoPin);
}

void loop() {
  servo.write(0);
  delay(1000);

  servo.write(180);

}