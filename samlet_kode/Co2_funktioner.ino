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

/*
// Calculate RS (sensor resistance)
float readRS() {
  float VC = 5.0;
  float rs = (calculateVolt() * MQ135.rl) / (VC - calculateVolt());
  return rs;
}
*/
float calculateRs(int samples = 80) {
  float RL = MQ135.rl;
  float VC = 4.95;
  float avg = 0.0f;

  for (int i = 0; i < samples; i++)
    avg += ((VC * RL) / readVoltage()) - RL;

  avg = avg / samples;
  return avg;
}
float readRSR0() {
  return (calculateRs(200) / MQ135.R0);
}

        float readPPM(float A, float B, float scale, float offset)
    {
        float ratio = readRSR0();

        float v = A * pow(ratio, B);  
        //float offset = A*ratio+B-scale*v;
        return scale * v + offset;             // scale converts curve value to real ppm me thinks
    }
