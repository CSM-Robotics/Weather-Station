#include "Geiger.h"

/*

This code isn't readable. I didn't find out that macros for all of the bit-twiddling I did exist in the Arduino header until I was most of the way through.

So I'll try and explain all the things the code is doing, and if someone wants to strip out all of my bad code and replace it with good code, go ahead. My code should work, as far as I can tell.

The code basically configures the microcontroller to read inputs from the geiger sensor and send them to an internal 16-bit counter without any interruption with the CPU at all. The CPU can then read the counter values at periodic intervals and send that value to the Raspi.

The first thing to happen is a particle strikes the radiation sensor.  The sensor will then drive the signal pin low for ~100 us.
(if noise is detected, the measurement is thrown away, as an interrupt will be triggererd that will clear out the counter.)

The signal pin will drive Pro RF pin 2 low, which triggers the External Interrupt Controller to generate an interrupt that in addition to calling isr() sends an "event" to the event system (EVSYS.)

The event system will then route the pulse to timer/counter 3 (TC3), which will increment.

The application code will then read out the counter values when appropriate.

TODO: explain clock generators and APBC stuff, as well as why things might crash now.

*/

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

void resetcounter() { // noise on the pin, so ignore previous measurements.
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

  // enable running in standby on TC3
  // set GCLKREQ to turn on only if there is an event to keep track of
  // reduce the clock from 48MHz to 32kHz or something

  // seems like EVSYS and EIC stay on while CPU turned off...?
  // figure out exactly what Arduino Low Power is doing and possibly replace it if it's interfering with our stuff.
  
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
  *count = (uint32_t)(*tc3_count); // TODO: tc3_count is actually a 16-bit number.  convert the packet header and stuff to reflect that.
  *tc3_count = 0; // clear counter
  
  return false;
}
