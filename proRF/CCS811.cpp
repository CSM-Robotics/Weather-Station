#include "CCS811.h"

CCS::CCS(unsigned char addr) : sensor(addr) { }

bool CCS::startSensor() {
  pinMode(wake_pin, OUTPUT);
  digitalWrite(wake_pin, 0); // start awake
  delay(1);
  bool err = !sensor.begin();
  digitalWrite(wake_pin, 1);
  delay(1);
  return err;
}

bool CCS::readSensor(float* CO2, float* tVOC) {
  digitalWrite(wake_pin, 0);
  delay(1); // wait for sensor to wake up, should be <1 ms
  while (!sensor.dataAvailable()) { // block until data is available
    //SerialUSB.println("busy waiting.");
  }
  sensor.readAlgorithmResults();
  *CO2 = sensor.getCO2();
  *tVOC = sensor.getTVOC();
  bool err = sensor.checkForStatusError(); // can't read status if sensor is powered down, so read now and then go to sleep
  digitalWrite(wake_pin, 1);
  delay(1); // wait for sensor to shut down, should be <1 ms
  return err;
}

bool CCS::setInfo(float hum, float tempC) {
  digitalWrite(wake_pin, 0);
  delay(1);
  sensor.setEnvironmentalData(hum, tempC);
  bool err = sensor.checkForStatusError();
  digitalWrite(wake_pin, 1);
  delay(1);
  return err;
}

// adjusted from Sparkfun example code
uint8_t CCS::getError() {  
  digitalWrite(wake_pin, 0);
  delay(1);
  uint8_t bits = sensor.getErrorRegister();
  if (bits == 0xFF) { // can't communicate with the CCS sensor at all
    bits = 0;
    bitSet(bits, 6);
    return bits;
  }
  bitClear(bits, 7); // there are only seven actual bits to set, so make sure the highest one is always cleared.
  digitalWrite(wake_pin, 1);
  delay(1);
  return bits;
}
