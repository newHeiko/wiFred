# Pre-compiled firmware updates for wiFred

## wiFred starting from rev0.6 (single controller)

- Use the \*-master.bin files

## wiFred rev 0.5x starting from 0.52 (two controllers)

- Use the \*esp12.bin files for the ESP12 and the \*esp12.hex files for the AVR

## wiFred rev 0.5 or rev0.51 (two controllers)

- Fix the PCB to match 0.52 or higher as following:
  - Remove SMD LED resistors R302, R303 and R304
  - Solder new wired resistors from the R302/R303/R304 pads to the ISP connector pins 1 and 3. See the documentation for pictures and a more detailed description
- Use the same firmwares as for rev0.52 mentioned above

## wiFred up to rev0.4x (two controllers)

- Compile AVR firmware from source from the esp12 branch, undefining WITH_FLASHLIGHT in led.h
- Use the \*esp12.bin files for the ESP12

