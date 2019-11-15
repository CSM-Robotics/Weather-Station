// sensors
#include"BME280.h"
#include "CCS811.h"
#include "Geiger.h"

// radio
#include "LoRa.h"

RH_RF95 rf95(12,6);

BME atmosphere;

void setup() {
  SerialUSB.begin(9600);
  while(!SerialUSB); // wait for serial lib to start up...

  SerialUSB.println("Hello!");

  atmosphere.startSensor(); // TODO: find out what sensors map to what pins
}

void loop() {
  // sleep for 30 sec.
  float atmosphereReadings[3];
  atmosphere.readSensor(atmosphereReadings);
  
  SerialUSB.println("Temp in C: ");
  SerialUSB.println(atmosphereReadings[0]);

  delay(2000);
  // send the stuff places over LoRa
  
}
