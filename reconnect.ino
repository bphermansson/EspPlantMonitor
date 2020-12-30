void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef DEBUG
      Serial.print("Attempting MQTT connection...");
    #endif

    // Attempt to connect
    if (client.connect(strDevice.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      #ifdef DEBUG
        String temp = "Connected to Mqtt broker as EspPlantMonitorV2";
        Serial.println(temp);
      #endif
    } else {
      Serial.print("Mqtt connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
