/* TODO: 
 *  implement checksums
 *  implement some sort of debug protocol
 *  check if RFM95W supports any kind of encryption
 *  get device to sleep in between readings (sleep for 1min, wake up, read instruments, send it, go back to sleep.
 *    only the geiger counter needs active monitoring (I think) and other sensors will continue to collect information?
 */

#include"BME280.h"
#include "CCS811.h"
#include "Geiger.h"

#include <RH_RF95.h>

RH_RF95 rf95(12,6);

BME atmosphere(0x77);
CCS airquality(0x5B);

struct weatherpacket {
  uint8_t nodeID;
  float tempC;
  float pressPa;
  float hum;
  float CO2ppm;
  float tVOCppb;
  uint16_t cpm;
  uint32_t checksum;
};

weatherpacket pack;

void setup() {
  SerialUSB.begin(9600);
  while (!SerialUSB);
  // don't wait for Serial lib to show up in final product, because this will hang the board if a USB cable isn't plugged in.
  
  SerialUSB.print("Initializing BME280... ");
  bool erratmos = atmosphere.startSensor();
  if (erratmos) {
    SerialUSB.println("Error initializing the atmospheric sensor!");
  } else {
    SerialUSB.println("done.");
  }

  SerialUSB.print("Initializing CCS811... ");
  bool errair = airquality.startSensor();
  if (errair) {
    SerialUSB.println("Error initializing the air quality sensor!");
  } else {
    SerialUSB.println("done.");
  }

  SerialUSB.print("Initializing RFM95W... ");
  if (!rf95.init()) {
    SerialUSB.println("Error initializing the radio!");
  } else {
    SerialUSB.println("done.");
  }
  rf95.setFrequency(915.0);

  pack.nodeID = 1;
}

void loop() {

  bool errread;
  
  errread = atmosphere.readSensor(&pack.tempC, &pack.pressPa, &pack.hum);
  if (errread) {
    SerialUSB.println("Error reading the atmospheric sensor!");
  }

  float alt;
  errread = atmosphere.readAlt(&alt);
  if (errread) {
    SerialUSB.println("Error reading the atmospheric sensor!");
  }

  // NOTE: the CCS811 sensor requires 20 mins of uptime to generate useful data.
  // and a 48-hour initial burn-in period.
  errread = airquality.readSensor(&pack.CO2ppm, &pack.tVOCppb);
  if (errread) {
    SerialUSB.println("Error reading the air quality sensor!");
  }

  bool errset = airquality.setInfo(pack.hum, pack.tempC);
  if (errset) {
    SerialUSB.println("Error setting air quality data!");
  }

  SerialUSB.print(pack.tempC);
  SerialUSB.print(" C, ");
  SerialUSB.print(pack.pressPa);
  SerialUSB.print(" Pa, "); // 1.0 atm = 101325 Pa
  SerialUSB.print(pack.hum);
  SerialUSB.print(" %, ");
  SerialUSB.print(alt);
  SerialUSB.print(" ft, ");
  SerialUSB.print(pack.CO2ppm);
  SerialUSB.print(" ppm CO2, ");
  SerialUSB.print(pack.tVOCppb);
  SerialUSB.println(" ppb tVOC");
  
  rf95.send(reinterpret_cast<uint8_t*>(&pack), sizeof(pack));
  rf95.waitPacketSent();
  
  delay(2000);
}
