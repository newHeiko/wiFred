# wiFred
wiThrottle-compatible hardware controller

This repo contains the first steps for three different, but closely related projects:

1) Battery-powered WiFi interface for cheap analog wall clocks to get their time from a JMRI json server
  - hardware in hardware/wfred_loconet_clock.* - same PCB as 3) below, only add the ESP12E, not the ATMega328P, not the loconet_interface...
2) Battery-powered wiThrottle-compatible throttle
  - hardware in hardware/wfred_rev2.*
3) Battery-powered Fred-to-wiThrottle interface
  - hardware in hardware/wfred_loconet_clock.* - same PCB as 1) above, don't add the clock connector

All three are built around ESP12E/ESP12F WiFi modules, powered by 2xAA cells and supposed to fit a Strapubox 6090 housing
(Fredi form factor, but includes a battery compartment) when the PCB is 0.8mm thick (or thinner).

The ESP source code consists of an Arduino sketch which contains the firmware for all three, configurable by a web interface (WIP).

Barely tested AVR source code available for wiThrottle-compatible throttle.

In-Progress-Version of documentation available in documentation/ subfolder.

Fred-to-wiThrottle interface sent to back burner, may never be done.

I have ordered PCBs around Christmas 2017 and hope to have a proof of concept plus more/better organized docs by summer (2018).

Also have a look at the pictures in this directory, they show 3D-renderings of the PCBs in early stages.

Please be patient with me if you have any questions, this is my first GitHub repo and I'm not a developer by trade, so I will make a lot of stupid mistakes ;)
