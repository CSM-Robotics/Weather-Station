/*

code to interface with the radiation sensor sold by Sparkfun.

since this module was not built by Sparkfun, there isn't an easy-to-use library for this. the code is complicated. see geiger.cpp for details on how it works.

(https://www.sparkfun.com/products/14209)

method description:

startSensor() - initializes the sensor. does not depend on I2C bus for IO, only digital input pins

readSensor(uint32_t* count) - sets count to the number of radioactive particles that have hit the sensor
(not sure if gamma counts as particles. whatever.)

*/

#include <Arduino.h>
#pragma once

class Geiger {
  public:
    bool startSensor();
	bool readSensor(uint32_t* count);
};
