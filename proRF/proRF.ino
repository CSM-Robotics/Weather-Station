// sensors
#include"BME280.h"
#include "CCS811.h"
#include "Geiger.h"

// radio
#include "LoRa.h"

BME b;

packet inProgress;

const int id = 1;

void setup() {
  b.startSensor();

  inProgress.nodeID = id;
	// set up all devices, some may require burn-in time to function properly
		// if device requires burn-in, don't block.
		// sleep until data is ready.
}

void loop() {
  float tempResults[3];
  b.readSensor(tempResults);

  inProgress.tempC = tempResults[tempC];
  inProgress.presskPa = tempResults[presskPa];
  inProgress.humPer = tempResults[humPer];

  
	// send everything
}
