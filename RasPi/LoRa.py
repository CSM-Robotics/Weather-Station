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

import datetime # for timestamps

import logging # for writing to a log file for debugging
import argparse

import json # for packing object data
import requests # for talking to the AWS server

packetlen = 36 + 4
totalpacketlen = packetlen + 4 # the actual C++ struct on the Arduino is 36 bytes, and we add 4 for the packet header

packettime = 2.0 # time in between packets in minutes

packettotal = 0 # packets understood

center_freq = 915.0 # in MHz

def packed_to_tuple(packed_data):
    '''converts a C++-style struct coming from the Arduino to a Python tuple for easier data processing.'''
    dataformat = namedtuple('dataformat', 'ActualNodeID NodeID tempC pressPa hum CO2 tVOC count packetcount deviceinfo')
    packetformat = '<IIfffffHxxII' # this is the format of a packet written in a format that struct can understand.
    return dataformat._make(struct.unpack(packetformat, packed_data))

def tuple_to_json(tuple_data):
    '''turns a Python tuple produced by packed_to_tuple() to a JSON object for submission to AWS.'''
    ret_dict = {}
    
    date_str =  str(datetime.datetime.utcnow().replace(tzinfo=datetime.timezone.utc).isoformat())
    ret_dict['date'] = date_str
    ret_dict['temp'] = tuple_data.tempC
    ret_dict['pressPa'] = tuple_data.pressPa
    ret_dict['hum'] = tuple_data.hum
    ret_dict['count'] = tuple_data.count / packettime # this is really the counts per minute, but the JSON that Tim's AWS server accepts calls it the count.
    ret_dict['packetNum'] = tuple_data.packetcount
    ret_dict['tvocpp'] = tuple_data.tVOC # the JSON item for this is misspelled.
    ret_dict['co2ppm'] = tuple_data.CO2
    return ret_dict

def parsepacket(pack):
    '''recieves a raw packet from the LoRa hardware and returns a dict containing (almost) the same data, or None if the packet was corrupted.'''
    if pack is None:
        return None

    # check that the packet is of the proper length
    if len(pack) != packetlen:
        print(len(pack))
        print('WARNING - packet is not of proper length, dropping it.')
        return None
    
    data = packed_to_tuple(pack)
    if data.deviceinfo != 0:
        logging.warning('packet deviceinfo field is %h, likely a measurement error.' % data.deviceinfo)
        print('WARNING - packet deviceinfo field is %h, likely a measurement error.' % data.deviceinfo)
        return None
    
    cpm = data.count / packettime
    
    return tuple_to_json(data)

def printpacket(data):
    '''prints out a packet with the relevant information.  Not required for operation, but really useful for debugging.'''
    global packettotal
    print(datetime.datetime.now().strftime('%m/%d/%y %I:%M:%S %p') + '    ', end='')
    temp_f = (9.0 / 5.0) * data['temp'] + 32.0
    print('host packet %d (client packet %d):    %4.2f C (%4.2f F), %4.2f Pa, %4.2f percent hum, %4.2f ppm CO2, %4.2f ppb tVOC, %4.2f cpm' % (packettotal, data['packetNum'], data['temp'], temp_f, data['pressPa'], data['hum'], data['co2ppm'], data['tvocpp'], data['count']))

def get_auth():
    '''returns a json auth token that we can use to upload data for the next 24 hours.'''
    auth_addr = 'https://api.csmrobotics.club/login'
    
    json_pass_file_path = '/home/pi/Desktop/Weather-Station-Sec/AWS_info.json' # convert the file to a Python object, then to a JSON-encoded string.
    with open(json_pass_file_path, 'r') as jpf:
        data = json.load(jpf)
    
    auth = requests.post(auth_addr, data = json.dumps(data))
    if (auth.status_code != 200):
        print('While making a login POST request, the AWS server gave an HTTP response code of %d' % auth.status_code)
        logging.critical('While making a login POST request, the AWS server gave an HTTP response code of %d' % auth.status_code)
        sys.exit(1)
    short_auth_header = {'Authorization': auth.headers['Authorization']}
    return short_auth_header

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='a LoRa listening script.')
    parser.add_argument('--logpath', type=str, default='/var/log/', help='path to file used to log errors and dropped packets')
    args = parser.parse_args()
    
    logname = 'LoRa.log' # important errors and events get recorded in the log for future data processing
    
    print('starting LoRa script...')
    logging.basicConfig(filename=args.logpath + logname, format='%(asctime)s %(levelname)s: %(message)s', datefmt='%m/%d/%y %I:%M:%S %p', level=logging.INFO)
    logging.info('starting LoRa script...')

    CS = DigitalInOut(board.CE1)
    RESET = DigitalInOut(board.D25)
    spi = busio.SPI(board.SCK_1, MOSI=board.MOSI_1, MISO=board.MISO_1) # using the second SPI bus, follow /boot/config.txt instructions above

    try:
        rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, center_freq) # use 915MHz since we're in the US and it isn't legal to broadcast on other frequencies
        
        rfm9x.enable_crc = True # enable checksums to protect data against accidental tampering
        # a CRC is not an encryption algorithm.
        
        rfm9x.ack_delay = 0.2

        rfm9x.node = 1
        rfm9x.destination = 2 # currently only have one weather station, so the literal is hard-coded.  We need to transmit "ack" packets in order to get reliable data transmission.

    except RuntimeError as error:
        logging.critical('failed to initialize the LoRa module!')
        print('RFM9X initialization error: ', error)
        sys.exit(1)

    print('configured radio for %.1f MHz' % center_freq)
    
    add_data_addr = 'https://api.csmrobotics.club/api/wss/addPackage'
    
    num_dropped_packets = 0
    measuring_dropped_packets = False # instead of recording every time we get a dropped packet, we record when we start and stop noticing dropped packets and extrapolate as to the number of packets we didn't see.
    # this method saves a lot of storage space, which matters when using SD cards for a while.
    
    short_auth_header = None
    last_timestamp = 0

    while True:
        current_time = time.time()
        if current_time - last_timestamp > 60 * 60 * 24:
            logging.info('updating auth token.')
            print('updating auth token!')
            short_auth_header = get_auth()
            last_timestamp = current_time
        # get a packet from the arduino, and send an ack packet in response.  Timeout after waiting about one packet transmission period.
        data = parsepacket(rfm9x.receive(with_ack=True, with_header=True, timeout=(packettime * 60) + 1))
        if data is None:
            if not measuring_dropped_packets:
                measuring_dropped_packets = True
                logging.warning('detected a dropped packet, checking for more...')
            num_dropped_packets += 1
        else:
            printpacket(data)
            packettotal += 1
            if measuring_dropped_packets:
                logging.warning('estimating a total of ' + str(num_dropped_packets) + ' dropped packets.')
                measuring_dropped_packets = False
                num_dropped_packets = 0
            add_data = requests.post(add_data_addr, headers = short_auth_header, json = data)
            if (add_data.status_code != 200):
                print('While making a data POST request, the AWS server gave an HTTP response code of %d' % add_data.status_code)
                logging.critical('While making a data POST request, the AWS server gave an HTTP response code of %d' % add_data.status_code)

