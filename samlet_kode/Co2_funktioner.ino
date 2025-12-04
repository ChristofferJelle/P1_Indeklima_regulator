
float calculateVolt(int samples = 50) {
  float avg = 0;

  for (int i = 0; i < samples; i++) {
    avg += analogRead(MQ135.co2Pin);
  }
  avg = avg / samples;


  return avg;
}

float readVoltage() {
  float ADC = MQ135.espResulution * (calculateVolt() / (pow(2.0, bits) - 1.0));

  float VoltageDivider = (1000.0 + 2000.0) / 2000.0;
  (ADC * VoltageDivider);
  MQ135.averageVolt = (ADC * VoltageDivider);
  return MQ135.averageVolt;
};

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
