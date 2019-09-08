#include <RH_RF95.h>

// [ temp float ] [ hum float ] [ pressure float ] [ tVOC int ] [ CO2 int ] [ Geiger int ] [ battery level float ]
struct packet {
  int nodeID; // unique id identifying the weather node in case we ever have >1
  float tempC;
  float presskPa;
  float humPer;
  int tVOCCount;
  int CO2Count;
  int geigerCount;
  float battLevel; // 0-1 indicating battery percentage
};

class LoRa {
  bool startRadio();
  bool sendPacket(const packet* p);
  bool recvPacket(const packet* p); // not sure if this is needed
};
