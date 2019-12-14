#include "Geiger.h"

const int signPin = 2;
const int noisePin = 3;

bool Geiger::startSensor() {
  pinMode(signPin, INPUT);
  digitalWrite(signPin, HIGH);

  pinMode(noisePin, INPUT);
  digitalWrite(noisePin, HIGH);
	return false;
}

bool Geiger::readSensor(float* readings) {
  
  // todo
  
	return false;
}
