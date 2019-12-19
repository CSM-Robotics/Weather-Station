#include "Geiger.h"

volatile uint16_t* tc3_ctla = (uint16_t*)(0x42002C00 + 0);
volatile uint16_t* tc3_readreq = (uint16_t*)(0x42002C00 + 0x2);
volatile uint8_t* tc3_ctlb = (uint8_t*)(0x42002C00 + 0x05);
volatile uint8_t* tc3_status = (uint8_t*)(0x42002C00 + 0x0F);
volatile uint16_t* tc3_count = (uint16_t*)(0x42002C00 + 0x10);
volatile uint16_t* tc3_event = (uint16_t*)(0x42002C00 + 0xA);

volatile uint32_t* port_dirclear = (uint32_t*)(0x41004400 + 0x4);
volatile uint8_t* port_pincfg = (uint8_t*)(0x41004400 + (0x40 + 14 * 0x01));
volatile uint8_t* port_periphmux = (uint8_t*)(0x41004400 + (0x30 + 14 * 0x01));

volatile uint32_t* apbbmask = (uint32_t*)(0x40000400 + 0x1C);
volatile uint32_t* apbcmask = (uint32_t*)(0x40000400 + 0x20);

volatile uint32_t* clkgencontrol = (uint32_t*)(0x40000C00 + 0x4);
volatile uint16_t* clkcontrol = (uint16_t*)(0x40000C00 + 0x2);
volatile uint8_t* clkstatus = (uint8_t*)(0x40000C00 + 0x1);

volatile uint8_t* evsyscontrol = (uint8_t*)(0x42000400 + 0x0);
volatile uint32_t* evsyschannel = (uint32_t*)(0x42000400 + 0x4);
volatile uint16_t* evsysuser = (uint16_t*)(0x42000400 + 0x8);

volatile uint8_t* eiccontrol = (uint8_t*)(0x40001800 + 0);
volatile uint8_t* eicstatus = (uint8_t*)(0x40001800 + 0x1);
volatile uint32_t* eicevent = (uint32_t*)(0x40001800 + 0x4);
volatile uint32_t* eicconfig = (uint32_t*)(0x40001800 + 0x18);

void isr() { } // dummy isr, makes things work...

void resetcounter() { // noise on the pin, so ignore previous measurements? - need to figure out how this works.
  *tc3_count = 0;
  SerialUSB.println("noise detected - clearing counts.");
}

bool Geiger::startSensor() {
  *port_dirclear |= (((uint32_t)1) << 14); // set pin PA14 to be an input
  *port_pincfg = (uint8_t)0x3; // enable input reading and let the peripheral use the pin
  *port_periphmux = (uint8_t)0x00; // let peripheral A control the pin (EXTINT14)
  
  *clkgencontrol = (uint32_t)0x10706; // 48MHz clock, enabled, clk gen 6
  while (*clkstatus >> 7 == 1);
  *clkcontrol = (uint16_t)0x461B; // enable GCLK6 with above source going to TC3
  *apbcmask |= (((uint32_t)1) << 11); // enable clock for tc3
  
  *clkcontrol = (uint16_t)0x4607; // enable GCLK8 with above source going to EVSYS channel 0
  *apbcmask |= (((uint32_t)1) << 1); // enable EVSYS clock
  
  *clkcontrol = (uint16_t)0x4605; // enable GCLK7 with above source going to EIC
  // apbc mask already set
  
  attachInterrupt(digitalPinToInterrupt(2), isr, FALLING);
  *eicevent |= ((uint32_t)1) << 14; // turn on external interrupt 14 to generate events
  
  *tc3_event = 0x2; // count on reception of event
    
  // select 16-bit counter mode
  *tc3_ctlb |= ((uint8_t)1) << 2; // select 1-shot operation
  while(*tc3_status >> 7 == 1);

  *tc3_event |= ((uint16_t)1) << 5; // turn on TCEI (the doc says it's only required for async operations, I think it's wrong)
  
  *tc3_ctla |= (((uint16_t)1) << 1); // enable tc3 peripheral
  while(*tc3_status >> 7 == 1);
  
  if (*tc3_status != 0) {
    SerialUSB.print("counter problematic, status register: ");
    SerialUSB.println(*tc3_status, BIN);
    return true;
  }

  *tc3_readreq = (uint16_t)0xC010; // request read syncronization with count register, use 0x8010 and ignore this code and update resetcounter isr if you don't want automatic syncronization.
  while (*tc3_status >> 7 == 1);

  *evsyscontrol |= (((uint8_t)1) << 4); // generic clocks for channels are always on.
  *evsyschannel = (uint32_t)0x81A0000; // connect EXTint14 to channel 0
  
  *evsysuser = (uint16_t)0x112; // connect channel 0 to TC3

  pinMode(3, INPUT);

  digitalWrite(2, HIGH); // probably turns on internal pullups...?
  digitalWrite(3, HIGH);
  
  attachInterrupt(digitalPinToInterrupt(3), resetcounter, RISING);
  return false;
}

bool Geiger::readSensor(uint32_t* count) {
  *count = (uint32_t)(*tc3_count);
  *tc3_count = 0; // clear counter
  
  return false;
}
