/*

CCS811 wrapper code - encloses all the stuff required to interface with the CCS811 sensor breakout from Sparkfun.
(https://www.sparkfun.com/products/14193)

method descriptions:

CCS(unsigned char addr) - configures the CCS sensor over the I2C bus with the address addr.

startSensor() - initializes the CCS sensor. returns true if an error occurred.

readSensor() - reads the equivalent CO2 reading in ppm, and the total Volatile Organic Compounds in the air in ppb.
note that this method blocks until data is available, but that should be fine since we're not doing anything else.

setInfo() - hum and tempC will be used to calibrate the CCS811 sensor in order to provide more accurate readings.
data comes from the BME280 sensor.

*/

#include <SparkFunCCS811.h>
#include <Arduino.h>
#pragma once

class CCS {
  public:
    CCS(unsigned char addr);
	  bool startSensor();
    bool readSensor(float* CO2, float* tVOC);
    bool setInfo(float hum, float tempC); // get data from a BME280 and use it to calibrate the sensor!
  private:
    CCS811 sensor;
};
