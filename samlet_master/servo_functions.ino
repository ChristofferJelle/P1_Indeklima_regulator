void ServoClose() {
  servo.write(0);
  servoState = sweepClose;
}

void ServoOpen() {
  servo.write(180);
  servoState = sweepOpen;
}

float ShuntVoltage() {
  int resistor = 1;
  float maxVolt = 3.3;

  //calculate average to minimise noise on analog pin
  float avgVolt = 0.0;
  int samples = 350;
  for (int i = 0; i < samples; i++) {
    float shuntVolt = maxVolt * analogRead(SHUNT_PIN) / (pow(2, 12) - 1.0);
    avgVolt += shuntVolt;
  }

  return avgVolt / samples;
}

