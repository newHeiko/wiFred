# wiFred
wiThrottle-compatible hardware controller

This repo contains schematics, PCB and documentation for the wiFred, a wiThrottle-compatible WiFi hardware controller developed to unchain the Fremo Fred [1] / Fredi [2] from its loconet cable.

![wiFred rev0.5](documentation/images/2021-01-23-preview0001.jpg)

### Branch information
Several versions are contained in different branches:

- master branch: Designed to fit into Strapubox 2090 housing, powered by a lithium battery, hardware: AVR ATMega328P plus ESP12F, SMD pushbuttons
  - rev0.4
  - rev0.5 (Added flashlight)
  - rev0.51 (Fixed silkscreen on PCB)
- esp32 branch: Designed to fit into Strapubox 2090 housing, powered by a lithium battery, hardware: ESP32-S2-WROOM, THT pushbuttons
  - will become rev0.6
- newAgeEnclosures branch: No longer developed, designed for New Age Enclosures Compact AA housing, powered by 2x AA cells, hardware: AVR ATMega328P plus ESP12F, SMD pushbuttons
- AABatteryPrototype branch: No longer developed, designed for Strapubox 6090 housing, powered by 2x AA cells, hardware: AVR ATMega328P plus ESP12F, SMD pushbuttons, very tight fit requires multiple modifications to Strapubox housing

[1]: <http://fremodcc.sourceforge.net/diy/fred/fred_e.html>
[2]: <http://fremodcc.sourceforge.net/diy/fred2/fredi_d.html>
