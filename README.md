# wiFred
wiThrottle-compatible hardware controller
x
This repo contains schematics, PCB and documentation for the wiFred, a wiThrottle-compatible WiFi hardware controller developed to unchain the Fremo Fred [1] / Fredi [2] from its loconet cable.

![wiFred rev0.5](documentation/images/2021-01-23-preview0001.jpg)

### Branch information
Several versions are contained in different branches:

- master branch: Designed to fit into Strapubox 2090 housing, powered by a lithium battery, hardware: ESP32-S2-WROOM, THT pushbuttons
  - rev0.6 (initial)
  - rev0.62 (switched USB connector to USB-C)
  - rev0.63 (replaced constant current source for flashlight with charge pump, replaced a few out-of-stock parts)
  - rev0.64 (added silk-screen text for LEDs and colored buttons, moved flashlight LEDs further apart, added twist protection for direction switch)
  - Latest Firmware: https://github.com/newHeiko/wiFred/blob/master/software/pre-compiled/2022-10-16-esp32-firmware.bin
- esp12 branch: Designed to fit into Strapubox 2090 housing, powered by a lithium battery, hardware: AVR ATMega328P plus ESP12F, SMD pushbuttons
  - rev0.4
  - rev0.5 (Added flashlight)
  - rev0.51 (Fixed silkscreen on PCB)
  - Latest Firmware: https://github.com/newHeiko/wiFred/blob/master/software/pre-compiled/2022-10-16-esp12-firmware.bin
- esp12-newAVR branch: Same hardware as esp12 branch, but with improved communication protocol between AVR and ESP12F.
  - Latest Firmware: https://github.com/newHeiko/wiFred/blob/master/software/pre-compiled/2022-10-16-esp12-newAVR-firmware.bin to go with: https://github.com/newHeiko/wiFred/blob/master/software/pre-compiled/2022-10-16-newAVR-firmware.hex
- newAgeEnclosures branch: No longer developed, designed for New Age Enclosures Compact AA housing, powered by 2x AA cells, hardware: AVR ATMega328P plus ESP12F, SMD pushbuttons
- AABatteryPrototype branch: No longer developed, designed for Strapubox 6090 housing, powered by 2x AA cells, hardware: AVR ATMega328P plus ESP12F, SMD pushbuttons, very tight fit requires multiple modifications to Strapubox housing

[1]: <http://fremodcc.sourceforge.net/diy/fred/fred_e.html>
[2]: <http://fremodcc.sourceforge.net/diy/fred2/fredi_d.html>
