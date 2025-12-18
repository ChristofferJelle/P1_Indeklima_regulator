void UpdateSensorData(bool direction, SensorDataLimitTp& LimitData) {
  switch (LimitData.CurrentSensorData) {
    case 'T':
      LimitData.Temp += direction ? -1 : 1;
      Serial.print("Temp: ");
      Serial.println(s1.Temp);
      break;
    case 'H':
      LimitData.Humid += direction ? -1 : 1;
      Serial.print("Humidity: ");
      Serial.println(s1.Humid);
      break;
    case 'C':
      LimitData.CO2 += direction ? -1 : 1;
      Serial.print("CO2: ");
      Serial.println(s1.CO2);
      break;
  }
}

void ReadEncoder() {
  currentStateCLK = digitalRead(CLK_PIN);
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    bool direction = digitalRead(DT_PIN) != currentStateCLK;
    if (screenStateTp == upperLimitValue) {
      UpdateSensorData(direction, s1);
    } else if (screenStateTp == lowerLimitValue) {
      UpdateSensorData(direction, s2);
    }

  }
  lastStateCLK = currentStateCLK;


  int buttonState = digitalRead(SW_PIN);

  // Check if the button state has changed (either pressed or released)
  if (buttonState != prevButtonSate) {
    buttonState = digitalRead(SW_PIN);

    // If the state is HIGH (meaning the button was just pressed DOWN)
    if (buttonState == HIGH) {
      switch (screenStateTp) {
        case main:
          // nuffin to press when on main screen
          ButtonPresses = 0;
          break;
        case upperLimitValue:
          ++ButtonPresses;
          switch (ButtonPresses) {
            case 1:
              s1.CurrentSensorData = 'H';
              Serial.println("set to change limit value of Humid");
              break;
            case 2:
              s1.CurrentSensorData = 'C';
              Serial.println("set to change upper limit value of co2");
              break;
            case 3:
              s1.CurrentSensorData = 'T';
              Serial.println("set to change upper limit value of Temp");
              ButtonPresses = 0; 
              break;
          }
          break;
        case lowerLimitValue:
          ++switchButtonPresses;
          switch (switchButtonPresses) {
            case 1:
              s2.CurrentSensorData = 'H';
              Serial.println("set to change lower limit value of Humid");
              break;
            case 2:
              s2.CurrentSensorData = 'T';
              Serial.println("set to change lower limit value of Temp");
              switchButtonPresses = 0;  
              break;
          }
          break;
      }
    }
    prevButtonSate = buttonState;
  }
}

void ChangeDisplay() {
  switchButtonState = digitalRead(switchMenuPin);

  if (switchButtonState == HIGH && switchPrevButtonSate == LOW) {
    Serial.print("DU TRYKKER PÅ MIN KNAAAAAAAAAAAP");
    Serial.print(screenStateTp);
    switch (screenStateTp) {
      case main:
        screenStateTp = upperLimitValue;
        break;
      case upperLimitValue:
        screenStateTp = lowerLimitValue;
        break;
      case lowerLimitValue:
        screenStateTp = main;
        break;
    }
  }
  switchPrevButtonSate = digitalRead(switchMenuPin);
}
void DrawupperLimitValues() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("UPPER THRESHHOLDS");

  tft.setCursor(0, 30);
  if (s1.CurrentSensorData == 'T') {
    tft.print("*");
  }
  tft.print("Temp Limit: ");
  tft.drawNumber(s1.Temp, tft.getCursorX(), tft.getCursorY());

  tft.setCursor(0, 60);
  if (s1.CurrentSensorData == 'H') {
    tft.print("*");
  }
  tft.print("Humid Limit: ");
  tft.drawNumber(s1.Humid, tft.getCursorX(), tft.getCursorY());

  tft.setCursor(0, 90);
  if (s1.CurrentSensorData == 'C') {
    tft.print("*");
  }
  tft.print("CO2 Limit: ");
  tft.drawNumber(s1.CO2, tft.getCursorX(), tft.getCursorY());
  tft.setTextSize(1.5);
    tft.setCursor(5, 125);
  tft.print("NOTE: SERVO/SENSOR FUNKTIONS 'PAUSED'");
}

void DrawLowerLimitValues() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("LOWER THRESHHOLDS");

  tft.setCursor(5, 30);
  if (s2.CurrentSensorData == 'T') {
    tft.print("*");
  }
  tft.print("Temp Limit: ");
  tft.drawNumber(s2.Temp, tft.getCursorX(), tft.getCursorY());

  tft.setCursor(0, 60);
  if (s2.CurrentSensorData == 'H') {
    tft.print("*");
  }
  tft.print("Humid Limit: ");
  tft.drawNumber(s2.Humid, tft.getCursorX(), tft.getCursorY());
  tft.setTextSize(1.5);
    tft.setCursor(5, 125);
  tft.print("NOTE: SERVO/SENSOR FUNKTIONS 'PAUSED'");
}
