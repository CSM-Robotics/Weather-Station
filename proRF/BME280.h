/*

SparkFun BME280 Sensor reading code

bool BME::startSensor() - starts up the sensor, and returns a boolean indicating whether setup was successful. (false result is failure)

bool BME::readSensor(float* readings) - read all four of the internal sensors.  This method is an argument of an array of >= 3 elements. The order in which sensor elements are written:
[ temp in C ] [ pressure in kPa ] [ altitude in m ] [ humidity in % ]

*/

#include <SparkFunBME280.h>
#include <Arduino.h>
#include <Wire.h>
#pragma once

enum BMEsensorValues {
  tempC = 0,
  presskPa = 1,
  humPer = 2,
};

class BME {
  public:
    BME(unsigned char addr);
	  bool startSensor();
    bool readSensor(float* temp, float* pres, float* hum);
    bool readAlt(float* result);
  private:
    BME280 sensor;
};
