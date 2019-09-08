#include "LoRa.h"

/*

packet structure:
  [ temp float ] [ hum float ] [ pressure float ] [ tVOC int ] [ CO2 int ] [ Geiger int ] [ battery level float ]

attach battery level to analog pins in order to detect when running out of power

*/

bool LoRa::startRadio() {
  return false;
}

bool LoRa::sendPacket(const packet* p) {
  return false;
}

bool LoRa::recvPacket(const packet* p) {
  return false;
}
