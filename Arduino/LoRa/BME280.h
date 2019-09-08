/*

SparkFun BME280 Sensor reading code

bool BME::startSensor() - starts up the sensor, and returns a boolean indicating whether setup was successful. (false result is failure)

bool BME::readSensor(float* readings) - read all four of the internal sensors.  This method is an argument of an array of >= 4 elements. The order in which sensor elements are written:
[ temp in C ] [ pressure in kPa ] [ altitude in m ] [ humidity in % ]

*/

#include "SparkFunBME280.h"
#pragma once

enum sensorValues {
  tempC = 0,
  preskPa,
  altm,
  humpc,
};

class BME {
  public:
    bool startSensor();
    bool readSensor(float* readings);
  private:
    BME280 sensor;
};
