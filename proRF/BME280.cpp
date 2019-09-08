#include "BME280.h"

bool BME::startSensor() {
  delay(10); // wait a little bit for the sensor to turn on
  uint8_t start = sensor.begin();
  return start == 0x60; // if the sensor does not return 0x60 then something went wrong
}

bool BME::readSensor(float* readings) {
  readings[tempC] = sensor.readTempC();
  readings[presskPa] = sensor.readFloatPressure();
  readings[humPer] = sensor.readFloatHumidity();

  return true; // no error checking here yet
}

bool BME::readAlt(float* result) { // seperate because altitude doesn't really change once the device is set up
    *result = sensor.readFloatAltitudeMeters();

    return true;
}
