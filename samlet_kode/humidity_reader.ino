float dhtRead() {
  //reads can take up to 250ms
  float humidity = dht.readHumidity();

  if (isnan(humidity)) { //check if any reads failed and exit early
    Serial.println("Failed to read humidity.");
    return 0;
  } else {
    return humidity;
  }
}
