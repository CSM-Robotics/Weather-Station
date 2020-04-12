#!/usr/bin/env python3

# RasPi config options needed before this script will run:

# sudo apt-get update && sudo apt-get upgrade
# sudo pip3 install --upgrade setuptools
# add the line 'dtoverlay=spi1-3cs' to the bottom of /boot/config.txt
# pip3 install RPI.GPIO
# pip3 install adafruit-blinka
# sudo pip3 install adafruit-circuitpython-rfm9x

# Vin -> 3.3v
# GND -> GND
# RFM G0 -> GPIO #5
# RFM RST -> GPIO #25
# RFM CLK -> SCK
# RFM MISO -> MISO
# RFM MOSI -> MOSI
# RFM CS -> CE1

import busio
import board
import adafruit_rfm9x

import struct # for data packing and unpacking
from collections import namedtuple # for converting into a Python dictionary

from digitalio import DigitalInOut, Direction, Pull

import sys
import signal
import time

import datetime # for timestamps

import logging # for writing to a log file for debugging

packetlen = 36
packettime = 2.0 / 60.0 # time in between packets in minutes

packettotal = 0 # packets understood

center_freq = 915.0

logname = './logfile_LoRa.txt'

def signal_handler(sig, frame):
    '''traps cntl-c to print out the number of received and understood packets'''
    
    global packettotal
    logging.info('caught cntl-c, exiting now.')
    print('\ngot %d packets.' % (packettotal)) 
    sys.exit(0)

def printpacket(pack):
    '''takes in a packet from the LoRa hardware and parses it, returning a formatted tuple with a variety of parameters'''
    if pack is None:
        return False

    # error checking to make sure that the packet is well-formed
    if len(pack) != packetlen:
        print('WARNING - packet is not of proper length, dropping it.')
        
        return False
    
    dataformat = namedtuple('dataformat', 'NodeID tempC pressPa hum CO2 tVOC count packetcount deviceinfo')
	
    packetformat = '<IfffffHxxII' # this is the format of a packet written in a format that struct can understand.
	
    data = dataformat._make(struct.unpack(packetformat, pack))
    
    global packettotal
    cpm = data.count / packettime
    print(datetime.datetime.now().strftime('%d/%m/%y %I:%M:%S') + '    ', end='')
    print('host packet %d (client packet %d):    %4.2f C, %4.2f Pa, %4.2f percent hum, %4.2f ppm CO2, %4.2f ppb tVOC, %4.2f cpm' % (packettotal, data.packetcount, data.tempC, data.pressPa, data.hum, data.CO2, data.tVOC, cpm))

    return True

if __name__ == '__main__':
    print('starting LoRa script...')
    logging.basicConfig(filename=logname, format='%(asctime)s %(levelname)s: %(message)s', datefmt='%d/%m/%y %I:%M:%S', level=logging.INFO)
    logging.info('starting LoRa script...')
    signal.signal(signal.SIGINT, signal_handler)

    CS = DigitalInOut(board.CE1)
    RESET = DigitalInOut(board.D25)
    spi = busio.SPI(board.SCK_1, MOSI=board.MOSI_1, MISO=board.MISO_1) # using the second SPI bus, follow /boot/config.txt instructions above
    
    try:
        rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, center_freq) # use 915MHz since we're in the US and it isn't legal to broadcast on other frequencies
        
        rfm9x.enable_crc = True # enable checksums to protect data against accidental tampering
        # a CRC is not an encryption algorithm.
        
        print('configured radio for %.1f MHz, %d dBm tx power' % (center_freq, rfm9x.tx_power))

        num_dropped_packets = 0
        measuring_dropped_packets = False

        while True:
            success = printpacket(rfm9x.receive(timeout = (packettime * 60) + 1))
            if not success:
                if not measuring_dropped_packets:
                    measuring_dropped_packets = True
                    logging.warning('detected a dropped packet, checking for more...')
                num_dropped_packets += 1
            else:
                packettotal += 1
                if measuring_dropped_packets:
                    logging.warning('detected a total of ' + str(num_dropped_packets) + ' dropped packets.')
                    measuring_dropped_packets = False
                    num_dropped_packets = 0

    except RuntimeError as error:
        logging.critical('failed to initialize the LoRa module!')
        print('RFM9X initialization error: ', error)

