#include <Arduino.h>
#pragma once

class Geiger {
  public:
    bool startSensor();
	  bool readSensor(uint32_t* count);
};
