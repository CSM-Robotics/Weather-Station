#include <Arduino.h>
#pragma once

class Geiger {
  public:
    bool startSensor();
	bool readSensor(float* readings);
  private:
    int cpm;
    int noise;
};
