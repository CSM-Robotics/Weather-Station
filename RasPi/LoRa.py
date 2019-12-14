#!/usr/bin/env python3

import busio
import board
import adafruit_rfm9x
import time

import struct # for data packing and unpacking
from collections import namedtuple # for converting into a Python dictionary

from digitalio import DigitalInOut, Direction, Pull

import sys
import signal

packetlen = 28

dropcount = 0
recvcount = 0

def signal_handler(sig, frame):
    print("\nsent %d packets, %d of which were dropped." % (recvcount, dropcount)) 
    sys.exit(0)

def parsepacket(pack):
    # error checking to make sure that the packet is well-formed
    if len(pack) != packetlen:
        print("Packet is not of proper length, dropping it.")
        
        global dropcount
        dropcount += 1
        
        return None
    
    dataformat = namedtuple('dataformat', 'NodeID tempC pressPa hum CO2 tVOC cpm')
    
    formatteddata = dataformat._make(struct.unpack('<Iffffff', pack))
    
    global recvcount # this is the weirdest thing...
    recvcount += 1

    return formatteddata

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)

    CS = DigitalInOut(board.CE1)
    RESET = DigitalInOut(board.D25)
    spi = busio.SPI(board.SCK_1, MOSI=board.MOSI_1, MISO=board.MISO_1) # using the second SPI bus, so you might need to add the line "dtoverlay=spi1-3cs" to /boot/config.txt

    try:
        rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, 915.0)
        rfm9x.tx_power = 23
        print('configuring radio for 915MHz, 23dBm tx power')
        while True:
            packet = rfm9x.receive()
            if packet is not None:
                data = parsepacket(packet)
                
                if data is not None:
                    print("%4.2f C, %4.2f Pa, %4.2f percent hum, %4.2f ppm CO2, %4.2f ppb tVOC, %4.2f cpm" % (data.tempC, data.pressPa, data.hum, data.CO2, data.tVOC, data.cpm))
    except RuntimeError as error:
        print('RFM9X initialization error: ', error)


