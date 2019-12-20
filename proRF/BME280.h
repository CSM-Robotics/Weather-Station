/*

BME280 wrapper code - encloses all the stuff required to inteface with the BME280 sensor breakout from Sparkfun.
(https://www.sparkfun.com/products/13676)

method descriptions:

BME(unsigned char addr) - configures the BME sensor over the I2C bus with the address addr.

startSensor() - initializes the BME sensor. returns true if an error occurred.

readSensor() - reads the temperature in C, pressure in pascals, and relative humidity in %. also returns true if an error occurred.

readAlt() - reads the altitude from the sensor. decoupled from everything else because we don't need to read the altitude that often, as it shouldn't change.

*/

#include <SparkFunBME280.h>
#include <Arduino.h>
#include <Wire.h>
#pragma once

class BME {
  public:
    BME(unsigned char addr);
	bool startSensor();
    bool readSensor(float* temp, float* pres, float* hum);
    bool readAlt(float* result);
  private:
    BME280 sensor;
};
