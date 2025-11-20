const int ntcPin = A1;

void setup() {
  Serial.begin(115200);
  pinMode(ntcPin, INPUT);
}

void loop() {
  float temp = NTCRead(true);
  Serial.print("temp = "); Serial.print(temp); Serial.println(" C");
  delay(1000);
}

float NTCRead(bool tempstate) {
  float rConst = 100000.0; //resistor in voltage divider
  float vSource = 4.98; //output voltage from Arduino
  float vOut = (analogRead(ntcPin) / 1023.0) * vSource; //output voltage from middle of voltage divider
  float ntc = (vOut * rConst) / (vSource - vOut); //calculate resistance of NTC

  //Steinhart-Hart coefficients from datasheet
  //for NTCLE100E3104JBO
  float A = 3.354016e-03;
  float B = 2.460382e-04;
  float C = 3.405377e-06;
  float D = 1.034240e-07;
  float R25 = 100000; //reference resistance

  float tempK = 1.0/( A + B*log(ntc/R25) + C*(pow(log(ntc/R25), 2.0)) + D*(pow(log(ntc/R25), 3.0)) ); //calculate temperature in K
  float tempC = tempK - 273.15; //calculate temperature in C

  if(tempstate) {
    return tempC;
  } else if(!tempstate) {
    return tempK;
  }
}
