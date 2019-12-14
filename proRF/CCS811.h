#include <SparkFunCCS811.h>
#include <Arduino.h>
#pragma once

enum CCSsensorValues {
	CO2count = 0,
	TVOCcount = 1,
};

class CCS {
  public:
    CCS(unsigned char addr);
	  bool startSensor();
    bool readSensor(float* CO2, float* tVOC);
    bool setInfo(float hum, float tempC); // get data from a BME280 and use it to calibrate the sensor!
  private:
    CCS811 sensor;
};
