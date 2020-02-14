/*

This is the main sketch for the Arduino side of the Weather Station project. It calls sensor libraries in order to get sensor readings, and sends them over LoRa to a Raspberry Pi.
    
	TODO: implement some sort of debug protocol - check battery levels and run a self-test
  TODO: get power consumption down, ideally <10mA

  POWER TODO:
  use sleep instead of idle and reconfigure Geiger.
  enable running in standby on TC3
  set GCLKREQ to turn on only if there is an event to keep track of - think about this.  make sure that the time to power up GCLK is short enough to record the radiation pulse (100us)
  set clock generator, clock, EVSYS, EIC, and TC3 to stay on if in sleep
  seems like EVSYS and EIC stay on while CPU turned off...?
  reduce the clock from 8MHz to 32kHz - fails at 32 for some reason

  reduce TX power if possible
  
  NOTE: with an 850mAh battery running the delay() loop, current is about 80mA.  This will exhaust the battery in about 10 hours with no solar recharge.

  Maintenance:
    The onboard LED will blink repeatedly if hardware fails to initialize properly.
    One blink means the BME sensor is problematic, two blinks is the CCS sensor, and three blinks is the radio module.
*/

#include"BME280.h"
#include "CCS811.h"
#include "Geiger.h"

#include <RH_RF95.h> // radiohead lib for LoRa communications

const unsigned char BMEADDR = 0x77;
const unsigned char CCSADDR = 0x5B;

RH_RF95 rf95(12,6);

BME atmosphere(BMEADDR);
CCS airquality(CCSADDR);
Geiger rad; // geiger sensor is not connected over I2C so no address required

uint32_t packetcounter = 0;

enum {
  BME_ERROR_BIT = 0,
  CCS_ERROR_BIT,
  GEIGER_ERROR_BIT, // not really implemented, hard to tell if something actually went wrong with the sensor
  RADIO_ERROR_BIT, // not implemented
  BATTERY_ERROR_BIT, // not implemented
  SOLAR_PANEL_ERROR_BIT, // not implemented
  CHARGING_STATUS_ERROR_BIT7 // not implemented
};

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

void setup() {
  SerialUSB.begin(9600);
  //while (!SerialUSB);
  // don't wait for Serial lib to show up in final product, because this will hang the board if a USB cable isn't plugged in.

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
  if (!rf95.init()) {
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
  
  errread = atmosphere.readSensor(&pack.tempC, &pack.pressPa, &pack.hum);
  if (errread) {
    bitSet(pack.deviceinfo, BME_ERROR_BIT);
    SerialUSB.println("Error reading the atmospheric sensor!");
  }

  float alt;
  errread = atmosphere.readAlt(&alt);
  if (errread) {
    bitSet(pack.deviceinfo, CCS_ERROR_BIT);
    SerialUSB.println("Error reading the atmospheric sensor!");
  }

  // NOTE: the CCS811 sensor requires 20 mins of uptime to generate useful data.
  errread = airquality.readSensor(&pack.CO2ppm, &pack.tVOCppb);
  if (errread) {
    bitSet(pack.deviceinfo, CCS_ERROR_BIT);
    SerialUSB.println("Error reading the air quality sensor!");
  }
  
  // use BME data to calibrate the CCS sensor.
  bool errset = airquality.setInfo(pack.hum, pack.tempC);
  if (errset) {
    bitSet(pack.deviceinfo, CCS_ERROR_BIT);
    SerialUSB.println("Error setting air quality data!");
  }
  
  errread = rad.readSensor(&pack.count);
  if (errread) {
    bitSet(pack.deviceinfo, GEIGER_ERROR_BIT);
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
  SerialUSB.print(BIN, pack.deviceinfo);
  SerialUSB.println();
  
  rf95.send(reinterpret_cast<uint8_t*>(&pack), sizeof(pack));
  rf95.waitPacketSent();

  rf95.sleep(); // turn the radio off for a while
  delay(2 * 1000);
  pack.deviceinfo = 0; // clear errors picked up for this packet
  // NOTE: the default arduino low power lib is broken, probably the fault of RTCZero?
}
