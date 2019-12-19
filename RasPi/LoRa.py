#!/usr/bin/env python3

import busio
import board
import adafruit_rfm9x

import struct # for data packing and unpacking
from collections import namedtuple # for converting into a Python dictionary

from digitalio import DigitalInOut, Direction, Pull

import sys
import signal
import time

packetlen = 32
packettime = 0.5 # time in between packets in minutes

packettotal = 0 # packets understood

center_freq = 915.0

def signal_handler(sig, frame):
    """traps cntl-c to print out the number of received and understood packets"""
    
    global packettotal
    print("\ngot %d packets." % (packettotal)) 
    sys.exit(0)

def parsepacket(pack):
    """takes in a packet from the LoRa hardware and parses it, returning a formatted tuple with a variety of parameters"""

    # error checking to make sure that the packet is well-formed
    if len(pack) != packetlen:
        print("WARNING - packet is not of proper length, dropping it.")
        
        return None
    
    dataformat = namedtuple('dataformat', 'NodeID tempC pressPa hum CO2 tVOC count packetcount')
    formatteddata = dataformat._make(struct.unpack('<IfffffII', pack))
    
    return formatteddata

if __name__ == '__main__':

    signal.signal(signal.SIGINT, signal_handler)

    CS = DigitalInOut(board.CE1)
    RESET = DigitalInOut(board.D25)
    spi = busio.SPI(board.SCK_1, MOSI=board.MOSI_1, MISO=board.MISO_1) # using the second SPI bus, so you might need to add the line "dtoverlay=spi1-3cs" to /boot/config.txt
    
    try:
        rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, center_freq) # use 915MHz since we're in America and it isn't legal to broadcast elsewhere I think
        
        rfm9x.tx_power = 23 # set max TX power
        rfm9x.enable_crc = True # enable checksums to protect data against accidental tampering
        
        print("configuring radio for %.1f MHz, %d dBm tx power" % (center_freq, rfm9x.tx_power))
        while True:
            packet = rfm9x.receive()
            if packet is not None:
                data = parsepacket(packet)
                if data is not None:
                    cpm = data.count / packettime
                    print("host packet %d (client packet %d):    %4.2f C, %4.2f Pa, %4.2f percent hum, %4.2f ppm CO2, %4.2f ppb tVOC, %4.2f cpm" % (packettotal, data.packetcount, data.tempC, data.pressPa, data.hum, data.CO2, data.tVOC, cpm))
                    packettotal += 1
                else:
                    print("packet is damaged.")
    except RuntimeError as error:
        print('RFM9X initialization error: ', error)


