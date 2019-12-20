#include "CCS811.h"

CCS::CCS(unsigned char addr) : sensor(addr) { }

bool CCS::startSensor() {
  return sensor.begin() != CCS811Core::SENSOR_SUCCESS;
}

bool CCS::readSensor(float* CO2, float* tVOC) {
  while (!sensor.dataAvailable()); // block until data is available
  sensor.readAlgorithmResults();
  *CO2 = sensor.getCO2();
  *tVOC = sensor.getTVOC();
  return sensor.checkForStatusError();
}

bool CCS::setInfo(float hum, float tempC) {
  sensor.setEnvironmentalData(hum, tempC);
  return sensor.checkForStatusError();
}
