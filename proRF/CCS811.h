#include <SparkFunCCS811.h>
#pragma once

enum CCSsensorValues {
	CO2count = 0,
	TVOCcount = 1,
};

class CCS {
  public:
    CCS(unsigned char addr);
	bool startSensor();
    bool readSensor(float* readings);
  private:
    CCS811 sensor;
};
