void ServoClose() {
  servo.write(0);
  servoState = SWEEP_CLOSE;
}

void ServoOpen() {
  servo.write(180);
  servoState = SWEEP_OPEN;
}

float ShuntCurrent() {
  float modstand = 1.0;
  //float shuntADC = analogRead(shuntPin);
  //float vOut = 3.33  * analogRead(shuntPin) / (pow(2.0, 12.0) - 1.0);
  float avgVolt = 0;
  int samples = 350;
  for (int i = 0; i < samples; i++) {
    avgVolt += 3.33  * analogRead(shuntPin) / (pow(2.0, 12.0) - 1.0);
  }
  avgVolt = avgVolt / samples;

  float current = avgVolt / modstand;
  return current;
}