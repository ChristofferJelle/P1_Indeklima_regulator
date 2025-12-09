void UpdateSensorData(bool direction, struct SensorDataLimitTp* dataStruct) {
  switch(dataStruct->currentSensorData) {
    case 'T':
      dataStruct->temp += direction ? -1 : 1;
      break;
    case 'H':
      dataStruct->humid += direction ? -1 : 1;
      break;
    case 'C':
      dataStruct->co2 += direction ? -1 : 1;
      break;
  }
}

void ReadEncoder() {
  currentStateCLK = digitalRead(CLK_PIN);
  if(currentStateCLK != lastStateCLK && currentStateCLK == 1) {
    bool direction = (digitalRead(DT_PIN) != currentStateCLK);
    UpdateSensorData(direction, &upperLimits);
  }
  lastStateCLK = currentStateCLK;

  int buttonState = digitalRead(SW_PIN);
  if(buttonState == HIGH && prevButtonState == LOW) {
    ++ButtonPresses;
  
    rotaryLastRefresh = millis();
    rotaryEncoderState = timeout;
    switch(ButtonPresses) {
      case 1:
        upperLimits.currentSensorData = 'H';
        Serial.println("Set to change limit value of humidity");
        break;
      case 2:
        upperLimits.currentSensorData = 'C';
        Serial.println("Set to change limit value of co2");
        break;
      case 3:
        upperLimits.currentSensorData = 'T';
        Serial.println("Set to change limit value of temperature");
        ButtonPresses = 0;
        break;
      default:
        break;
    }
  }
  prevButtonState = digitalRead(SW_PIN);
}

void DrawLimitValues() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("CHANGING THRESHHOLDS");

  tft.setCursor(0, 30);
  if(upperLimits.currentSensorData == 'T') {
    tft.print("*");
  }
  tft.print("Temperature Limit: ");
  tft.drawNumber(upperLimits.temp, tft.getCursorX(), tft.getCursorY());
  tft.print(" "); tft.print(176); tft.print("C");

  tft.setCursor(0, 60);
  if(upperLimits.currentSensorData == 'H') {
    tft.print("*");
  }
  tft.print("Humidity Limit: ");
  tft.drawNumber(upperLimits.humid, tft.getCursorX(), tft.getCursorY());
  tft.print("%");

  tft.setCursor(0, 90);
  if(upperLimits.currentSensorData == 'C') {
    tft.print("*");
  }
  tft.print("CO2 Limit: ");
  tft.drawNumber(upperLimits.co2, tft.getCursorX(), tft.getCursorY());
  tft.print(" ppm");
}

void InterruptCallback() {
  if(rotaryEncoderState != timeout
  && millis() - rotaryLastRefresh >= rotaryRefreshInterval) {
    rotaryLastRefresh = millis();
    rotaryEncoderState = timeout;
  }
}