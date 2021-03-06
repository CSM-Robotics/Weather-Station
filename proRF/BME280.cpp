#include "BME280.h"

BME::BME(unsigned char addr) : sensor() {
	sensor.settings.commInterface = I2C_MODE;
	sensor.settings.I2CAddress = addr;
}

bool BME::startSensor() {
  delay(10); // wait a little bit for the sensor to turn on
  
  sensor.settings.runMode = 3; // Normal mode??
  sensor.settings.tStandby = 0;
  sensor.settings.filter = 0;
 
  sensor.settings.tempOverSample = 1; // don't oversample anything
  sensor.settings.pressOverSample = 1;
  sensor.settings.humidOverSample = 1;

  delay(10);
  uint8_t start = sensor.begin();
  return start != 0x60; // if the sensor does not return 0x60 then something went wrong
}

bool BME::readSensor(float* temp, float* pres, float* hum) {
  *temp = sensor.readTempC();
  *pres = sensor.readFloatPressure();
  *hum = sensor.readFloatHumidity();

  return false; // the BME sensor lib doesn't have any way to determine if an error occurred.
}

bool BME::readAlt(float* result) {
  *result = sensor.readFloatAltitudeFeet();

  return false; // the BME sensor lib doesn't have any way to determine if an error occurred.
}
