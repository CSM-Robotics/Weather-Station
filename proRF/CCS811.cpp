#include "CCS811.h"

CCS::CCS(unsigned char addr) : sensor(addr) {

}

bool CCS::startSensor() {
  return sensor.begin() != 0;
}

bool CCS::readSensor(float* readings) {
  if (sensor.dataAvailable()) {
    sensor.readAlgorithmResults();
	readings[CO2count] = sensor.getCO2();
	readings[TVOCcount] = sensor.getTVOC();
  }
  return sensor.checkForStatusError() == true;
}
