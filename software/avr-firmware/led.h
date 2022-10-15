/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018  Heiko Rosemann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 *
 * This file provides functions and data structures for reading and writing LED values
 */

#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>

#define LED_FORWARD 0
#define LED_REVERSE 1
#define LED_STOP    2

typedef struct
{
  uint8_t onTime;
  uint8_t cycleTime;
  bool ledStatus;
} ledInfo;

/**
 * If this define is enabled, the flashlight LED will be lit on pushing the SHIFT key
 * This will light the STOP LED if used on prototype hardware
 */
// #define WITH_FLASHLIGHT

/**
 * Array if information about the LEDs
 */
extern volatile ledInfo LEDs[3];

/**
 * Initialize LED output ports
 */
void initLEDs(void);

/**
 * Turn LED on
 *
 * Parameter: i [0..2]: LED to turn on
 */
void setLEDoutput(uint8_t led);
/**
 * Turn LED off
 *
 * Parameter: i [0..2]: LED to turn off
 */
void clearLEDoutput(uint8_t led);

#endif
