#include "BME280.h"

/*

packet structure:
	[ temp float ] [ hum float ] [ pressure float ] [ tVOC int ] [ CO2 int ] [ Geiger int ] [ battery level float ]

attach battery level to analog pins in order to detect when running out of power

*/

BME b;

void setup() {
  b.startSensor();
  
	// set up all devices, some may require burn-in time to function properly
		// if device requires burn-in, don't block.
		// sleep until data is ready.
}

void loop() {
	// get BME data over I2C - actually don't think this needs any translation code
	// get CCS data over I2C - will need translation, want the sensor to interrupt when data is ready
	// get Gieger data over analog pins - will need translation, do math when ready

	// send everything
}
