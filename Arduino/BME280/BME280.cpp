bool BME::startSensor() {
  delay(10); // wait a little bit for the sensor to turn on
  uint8_t start = sensor.begin();
  return start == 0x60; // if the sensor does not return 0x60 then something went wrong
}

bool BME::readSensor(float* readings) {
  readings[tempC] = sensor.readTempC();
  readings[preskPa] = sensor.readFloatPressure();
  readings[altm] = sensor.readFloatAltitudeMeters();
  readings[humpc] = sensor.readFloatHumidity();
}
