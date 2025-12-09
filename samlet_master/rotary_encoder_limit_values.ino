void UpdateSensorData(bool direction) {
  switch (s1.CurrentSensorData) {
    case 'T':
      s1.Temp += direction ? -1 : 1;
      Serial.print("Temp: ");
      Serial.println(s1.Temp);
      break;
    case 'H':
      s1.Humid += direction ? -1 : 1;
      //Serial.print("Humidity: ");
      //Serial.println(s1.Humid);
      break;
    case 'C':
      s1.CO2 += direction ? -1 : 1;
      //Serial.print("CO2: ");
      //Serial.println(s1.CO2);
      break;
  }
}

void ReadEncoder() {
  currentStateCLK = digitalRead(CLK_PIN);
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    bool direction = digitalRead(DT_PIN) != currentStateCLK;
    UpdateSensorData(direction);
    //Serial.print("GET ME OUTTTTTTTTTT OF THIS STATE");
  }
  lastStateCLK = currentStateCLK;

  int buttonState = digitalRead(SW_PIN);

  if (buttonState == HIGH && prevButtonSate == LOW) {
    ++ButtonPresses;
    // GO INTO TIMEOUT STATE LITTERLY JUST SO THE DISPLAY CHANGES
    rotaryLastRefresh = millis();
    rotaryEncoderState = timeout;
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
  prevButtonSate = digitalRead(SW_PIN);
}

void DrawLimitValues() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("CHANGING THRESHHOLDS");

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
}

void InterruptCallback() {
  if (rotaryEncoderState != timeout && millis() - rotaryLastRefresh >= rotaryRefreshInterval) {
    rotaryLastRefresh = millis();
    rotaryEncoderState = timeout;
  }
}