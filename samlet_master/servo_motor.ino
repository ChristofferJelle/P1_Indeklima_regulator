void ServoClose(unsigned long startTime) {
  if (servo.read() > 0) {
    moveServo(startTime, 180, 0);  //go from 180 to 0 degrees
    servoState = sweepClose;
  }
}

void ServoOpen(unsigned long startTime) {
  if (servo.read() < 178) {
    moveServo(startTime, 0, 180);  //go 0 to 180 degrees
    servoState = sweepOpen;
  }
}

void moveServo(unsigned long startTime, int startAngle, int stopAngle) {
  unsigned long progress = millis() - startTime;

  if (progress <= servoActionTime) {
    long angle = map(progress, 0, servoActionTime, startAngle, stopAngle);
    servo.write(angle);
  }
}

float ShuntCurrent() {
  float modstand = 1.0;
  //float shuntADC = analogRead(shuntPin);
  //float vOut = 3.33  * analogRead(shuntPin) / (pow(2.0, 12.0) - 1.0);
  float avgVolt = 0;
  int samples = 350;
  for (int i = 0; i < samples; i++) {
    avgVolt += 3.33 * analogRead(SHUNT_PIN) / (pow(2.0, 12.0) - 1.0);
  }
  avgVolt = avgVolt / samples;

  float current = avgVolt / modstand;
  return current;
}
