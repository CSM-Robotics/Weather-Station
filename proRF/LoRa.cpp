#include "LoRa.h"

/*

packet structure:
  [ temp float ] [ hum float ] [ pressure float ] [ tVOC int ] [ CO2 int ] [ Geiger int ] [ battery level float ]

attach battery level to analog pins in order to detect when running out of power

*/

const int CS = 12;
const int interrupt = 6;
const float freq = 921.2f;

// assume that Serial lib is set up
LoRa::LoRa() : radio(CS, interrupt) {
  if(!radio.init()) {
    SerialUSB.println("ERROR: radio setup failed!");
  }
  radio.setFrequency(freq);
}

bool LoRa::sendPacket(const packet* p) {
  
  
  return false;
}

// recvPacket is non-blocking in order to allow for the microcontroller to sleep while nothing is happening
bool LoRa::recvPacket(const packet* p) {
  
  
  return false;
}
