float readVoltage() {
  float ADC = MQ135.espResulution * (analogRead(MQ135.co2Pin) / (pow(2.0, bits) - 1.0));

  float VoltageDivider = (1000.0 + 2000.0) / 2000.0;

  return (ADC * VoltageDivider);
};

float calculateVolt(int samples = 10) {
  float avg = 0;

  for (int i = 0; i < samples; i++) {
    avg += readVoltage();
  }
  avg = avg / samples;

  MQ135.averageVolt = avg;
  return MQ135.averageVolt;
}


// Calculate RS (sensor resistance)
float readRS() {
  float VC = 5.0;
  float rs = (calculateVolt() * MQ135.rl) / (VC - calculateVolt());
  return rs;
}

float readRSR0() {
  return (readRS() / MQ135.R0);
}

float readPPM(float a, float b) {
  return a * pow(readRSR0(), b);
}
