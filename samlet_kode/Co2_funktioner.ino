float calculateVolt(int samples = 10) {
  float avg = 0;

  for (int i = 0; i < samples; i++)
    avg += analogRead(MQ135.co2Pin);

  avg = avg / samples;

  MQ135.averageVolt = MQ135.resulution * (avg / (pow(2.0, bits) - 1.0));
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
  float VC = MQ135.resulution;
  float avg = 0.0f;
   float avgVolt = calculateVolt(samples);
   
 
        for(int i = 0; i < samples; i++)
            avg += ((VC*RL)/avgVolt)-RL;

        avg = avg / samples;
  return avg;
}
float readRSR0() {
  return (calculateRs(200) / MQ135.R0);
}

float readPPM(float A, float B, float scale, float offset) {
  float ratio = readRSR0();

  float v = A * pow(ratio, B);
  //float offset = A*ratio+B-scale*v;
  return scale * v + offset;  // scale converts curve value to real ppm me thinks
}
