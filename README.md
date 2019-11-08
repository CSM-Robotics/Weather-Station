# Weather-Station
This is the GitHub for the CSM Weather station project.

The goal for this weather station is to collect pressure/temperature/humidity/VOC/radiation measurements from an Arduino system, send it to a Raspberry Pi over LoRa, and publish that data somewhere in the cloud for other people to view.  Measurements will also be stored over time so that people can view graphs of the data.

Here is the [link](https://docs.google.com/spreadsheets/d/1q3k0UVijBZFMGUKlG-Q5725P6HpbFjzFsUlXNikVg40/edit?usp=sharing) to the BOM (Bill of Materials) for everything.

## Arduino information
The Arduino system uses multiple sensors to collect data, all of which are listed below.  For more detailed information check the Sparkfun "Getting Started" guides for the respective sensors.
- The [BME280](https://www.sparkfun.com/products/13676) sensor is a combination pressure/temperature/humidity sensor. (3.3v **only**!!)
- The [CCS811](https://www.sparkfun.com/products/14193) sensor measures air quality levels. (3.3v **only**!!)
- The [Pocket Geiger Sensor](https://www.sparkfun.com/products/14209) measures beta and gamma radiation! (3.3v /  5v compatible)

The CCS811 and BME280 sensors should be connected by I2C to the Pro RF.

Additionally, the Arduino itself isn't just an Uno derivative.  It's actually a [Sparkfun Pro RF](https://www.sparkfun.com/products/14916), which is **only** 3.3v compatible and has a couple more features.
The weather station also has a [Sunny Buddy](https://www.sparkfun.com/products/12885) for managing the solar panel and LiPo battery.

## Raspberry Pi information
The [Raspberry Pi Zero W](https://www.sparkfun.com/products/14277) uses an [RFM95W](https://www.adafruit.com/product/3072) in order to recieve data from the Arduino over LoRa.  It will then send data over WiFi to some kind of cloud server.

## Cloud information
Not a lot yet.

## Kyle's Rules
- If you have previous C/C++ experience, please don't write bad code and ask me to look at it.
- If you don't have previous C/C++ experience, feel free to ask questions about what the above phrase means.
- If you want to add code to this repository, you must have a way to prove to me it works before I'll look at it.  (Not asking for bug-free code, but please run it >1 time(s))
- Once your code works, either create a Pull Request or send the .ino files to me via email or something.
