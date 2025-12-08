void InitRotaryEncoder() {
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  attachInterrupt(CLK_PIN, InterruptCallback, FALLING);

  prevButtonSate = digitalRead(SW_PIN);

  Serial.println(prevButtonSate);

  //Read the initial state of CLK
  lastStateCLK = digitalRead(CLK_PIN);
  currentStateCLK = digitalRead(CLK_PIN);
}

void UpdateSensorData(bool direction) {
  switch(upperLimits.currentSensorData) {
    case 'T':
      upperLimits.temp += direction ? -1 : 1;
      break;
    case 'H':
      upperLimits.humid += direction ? -1 : 1;
      break;
    case 'C':
      upperLimits.co2 += direction ? -1 : 1;
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
        upperLimits.currentSensorData = 'H';
        Serial.println("set to change limit value of humid");

        break;
      case 2:
        upperLimits.currentSensorData = 'C';
        Serial.println("set to change limit value of co2");
        break;
      case 3:
        upperLimits.currentSensorData = 'T';
        Serial.println("set to change limit value of temp");
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
  if (upperLimits.currentSensorData == 'T') {
    tft.print("*");
  }
  tft.print("Temp Limit: ");
  tft.drawNumber(upperLimits.temp, tft.getCursorX(), tft.getCursorY());  

  tft.setCursor(0, 60);
  if (upperLimits.currentSensorData == 'H') {
    tft.print("*");
  }
  tft.print("Humid Limit: ");
  tft.drawNumber(upperLimits.humid, tft.getCursorX(), tft.getCursorY());

  tft.setCursor(0, 90);
  if (upperLimits.currentSensorData == 'C') {
    tft.print("*");
  }
  tft.print("CO2 Limit: ");
  tft.drawNumber(upperLimits.co2, tft.getCursorX(), tft.getCursorY());
}

void InterruptCallback() {
  if (rotaryEncoderState != timeout && millis() - rotaryLastRefresh >= rotaryRefreshInterval) {
    rotaryLastRefresh = millis();
    rotaryEncoderState = timeout;
  }
}