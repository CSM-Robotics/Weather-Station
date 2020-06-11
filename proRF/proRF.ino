/*

This is the main sketch for the Arduino side of the Weather Station project. It calls sensor libraries in order to get sensor readings, and sends them over LoRa to a Raspberry Pi.

  POWER TODO:
    reduce TX power if possible
    reduce idle power
  
  Maintenance:
    The onboard LED will blink repeatedly if hardware fails to initialize properly.
    One blink means the BME sensor is problematic, two blinks is the CCS sensor, and three blinks is the radio module.
*/

#include"BME280.h"
#include "CCS811.h"
#include "Geiger.h"

#include <RH_RF95.h> // radiohead lib for LoRa communications
#include <RHReliableDatagram.h>

const unsigned int packet_delay = 120; // delay between packets in seconds - currently two minutes
const unsigned int dbg_packet_delay = 2;

const unsigned char BMEADDR = 0x77;
const unsigned char CCSADDR = 0x5B;

const int ARDUINO_ADDRESS = 2;
const int RASPI_ADDRESS = 1;

RH_RF95 rf95(12,6);
RHReliableDatagram manager(rf95, ARDUINO_ADDRESS);

BME atmosphere(BMEADDR);
CCS airquality(CCSADDR);
Geiger rad; // geiger sensor is not connected over I2C, so no address is required

uint32_t packetcounter = 0;

struct weatherpacket { // device info is to tell the RasPi about the health of the device (1 if error, 0 if no error)
  const uint32_t nodeID;
  float tempC;
  float pressPa;
  float hum;
  float CO2ppm;
  float tVOCppb;
  uint16_t count;
  uint32_t packetnum;
  uint32_t deviceinfo;
};

weatherpacket pack = { 1 }; // set node ID to be 1 (each node ID is unique)

// if DEBUG_MODE is defined, the system will not boot up without being connected to a USB port (to avoid missing anything coming over the Serial port)
// and will emit packets every 2 seconds.
#define DEBUG_MODE false

void setup() {
  SerialUSB.begin(9600);
  if (DEBUG_MODE) {
    while (!SerialUSB);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  SerialUSB.print("Initializing BME280... ");
  bool erratmos = atmosphere.startSensor();
  if (erratmos) {
    SerialUSB.println("Error initializing the atmospheric sensor, blinking pattern.");
    while(true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  } else {
    SerialUSB.println("done.");
  }

  SerialUSB.print("Initializing CCS811... ");
  bool errair = airquality.startSensor();
  if (errair) {
    SerialUSB.println("Error initializing the air quality sensor, blinking pattern.");
    while(true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  } else {
    SerialUSB.println("done.");
  }

  SerialUSB.print("Initializing Geiger sensor... ");
  bool raderr = rad.startSensor();
  if (raderr) {
    SerialUSB.println("Error initializing the radiation sensor!");
  } else {
    SerialUSB.println("done.");
  }
  
  SerialUSB.print("Initializing RFM95W... ");
  if (!manager.init()) {
    SerialUSB.println("Error initializing the radio, blinking pattern");
    while(true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  } else {
    SerialUSB.println("done.");
  }
  
  rf95.setFrequency(915.0);
  rf95.setTxPower(23); // set max TX power
}

void loop() {  
  bool errread;
  
  atmosphere.readSensor(&pack.tempC, &pack.pressPa, &pack.hum); // BME sensor does not report errors.

  float alt;
  atmosphere.readAlt(&alt);

  // NOTE: the CCS811 sensor requires 20 mins of uptime to generate useful data.
  errread = airquality.readSensor(&pack.CO2ppm, &pack.tVOCppb);
  if (errread) {
    pack.deviceinfo |= airquality.getError();
    SerialUSB.println("Error reading the air quality sensor!");
  }
  
  // use BME data to calibrate the CCS sensor.
  bool errset = airquality.setInfo(pack.hum, pack.tempC);
  if (errset) {
    pack.deviceinfo |= (airquality.getError() << 8);
    SerialUSB.println("Error setting air quality data!");
  }
  
  errread = rad.readSensor(&pack.count);
  if (errread) {
    SerialUSB.println("Error reading the geiger sensor!");
  }

  pack.packetnum = packetcounter++; // packet counter to help detect dropped packets, ok if this rolls over in ~8000 years (assuming a packet every 1 min)

  SerialUSB.print("packet ");
  SerialUSB.print(pack.packetnum);
  SerialUSB.print(":   ");
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
  SerialUSB.print(" ppb tVOC, ");
  SerialUSB.print(pack.count);
  SerialUSB.print(" counts, error bitfield: ");
  SerialUSB.print(pack.deviceinfo, BIN);
  SerialUSB.println();
  
  manager.sendtoWait(reinterpret_cast<uint8_t*>(&pack), sizeof(pack), RASPI_ADDRESS);
  
  rf95.sleep(); // turn the radio off for a while
  if (DEBUG_MODE) {
    delay(dbg_packet_delay * 1000);
  } else {
    delay(packet_delay * 1000);
  }
  pack.deviceinfo = 0; // clear errors picked up for this packet
  // NOTE: the default arduino low power lib is broken, probably the fault of RTCZero?
  // also can't use WDT lib. :/
}
