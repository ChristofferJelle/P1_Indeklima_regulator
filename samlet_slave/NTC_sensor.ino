float NTCRead(bool tempstate) {
  float rConst = 100000.0;                                             //resistor in voltage divider
  float vSource = 3.33;                                                //output voltage from Arduino
  //float vOut = vSource * analogRead(ntcPin) / (pow(2.0, bits) - 1.0);  //output voltage from middle of voltage divider



  float avgVolt = 0;
  int samples = 35;
  for (int i = 0; i < samples; i++) {
    avgVolt += vSource * analogRead(ntcPin) / (pow(2.0, bits) - 1.0);
  }
  avgVolt = avgVolt / samples;

  float ntc = (avgVolt * rConst) / (vSource - avgVolt);  //calculate resistance of NTC

  /*
  Serial.print("analogReadNTC: \t ");
  float adc = analogRead(ntcPin);
  Serial.print(adc);
  Serial.print("\t");
  //  vOut = 3.3*adc/1023;
  Serial.println(avgVolt);
  */

  //Steinhart-Hart coefficients from datasheet
  //for NTCLE100E3104JBO
  float A = 3.354016e-03;
  float B = 2.460382e-04;
  float C = 3.405377e-06;
  float D = 1.034240e-07;
  float R25 = 100000;  //reference resistance

  float tempK = 1.0 / (A + B * log(ntc / R25) + C * (pow(log(ntc / R25), 2.0)) + D * (pow(log(ntc / R25), 3.0)));  //calculate temperature in K
  float tempC = tempK - 273.15;

  //Serial.print("temp: \t ");
  //Serial.print(tempC);  //calculate temperature in C

  if (tempstate) {
    return tempC-2;
  }
  return tempK-2;
}
