void UpdateSensorData(bool direction) {
  switch (s1.CurrentSensorData) {
    case 'T':
      s1.Temp += direction ? -1 : 1;
      Serial.print("Temp: ");
      Serial.println(s1.Temp);
      break;
    case 'H':
      s1.Humid += direction ? -1 : 1;
      Serial.print("Humidity: ");
      Serial.println(s1.Humid);
      break;
    case 'C':
      s1.CO2 += direction ? -1 : 1;
      Serial.print("CO2: ");
      Serial.println(s1.CO2);
      break;
  }
}

void readEncoder() {
  currentStateCLK = digitalRead(CLK);
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    bool direction = digitalRead(DT) != currentStateCLK;
    UpdateSensorData(direction);
  }
  lastStateCLK = currentStateCLK;

  int buttonState = digitalRead(SW);

  if (buttonState == HIGH && prevButtonSate == LOW) {
    ++ButtonPresses;

    switch (ButtonPresses) {
      case 1:
        s1.CurrentSensorData = 'H';
        Serial.println("set to change limit value of Humid");

        break;
      case 2:
        s1.CurrentSensorData = 'C';
        Serial.println("set to change limit value of co2");
        break;
      case 3:
        s1.CurrentSensorData = 'T';
        Serial.println("set to change limit value of Temp");
        ButtonPresses = 0;
        break;
      default:
        break;
    }
  }
  prevButtonSate = digitalRead(SW);
}
