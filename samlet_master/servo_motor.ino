void ServoClose() {
  servo.write(0);
}

void ServoOpen() {
  servo.write(180);
}

float ShuntCurrent(){
float modstand = 1;
float shuntADC = analogRead(shuntPin);
float vOut = 5 * shuntADC / (pow(2,12)-1);
float current = vOut/modstand;
return current;
}