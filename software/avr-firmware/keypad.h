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
 * This file provides functions for key input handling.
 *
 * The code for de-bouncing has originally been developed by Peter Dannegger
 * and can be found i.e. at https://www.avrfreaks.net/projects/efficient-key-debounce
 * This code has been extended to cover more than one input port and to include
 * key release detection.
 */

#include <stdint.h>

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#define KEY_REVERSE (1<<PB0)
#define KEY_FORWARD (1<<PB1)
#define KEY_ESTOP   (1<<PB3)
#define KEY_SHIFT   (1<<PB4)
#define KEY_SHIFT2  (1<<PB5)
#define KEY_F0      (1<<(PC0+8))
#define KEY_F1      (1<<(PC1+8))
#define KEY_F2      (1<<(PC2+8))
#define KEY_F3      (1<<(PC3+8))
#define KEY_F4      (1<<(PC4+8))

#define KEY_ALL     KEY_FORWARD | KEY_REVERSE | KEY_ESTOP | KEY_SHIFT | KEY_SHIFT2 | KEY_F0 | KEY_F1 | KEY_F2 | KEY_F3 | KEY_F4

/**
 * Return current key status
 */
uint16_t getKeyState(uint16_t keyMask);

/**
 * Return new key presses since last call
 */
uint16_t getKeyPresses(uint16_t keyMask);

/**
 * Return new key releases since last call
 */
uint16_t getKeyReleases(uint16_t keyMask);

/**
 * To be called every 10ms from ISR - handle key input
 */
void debounceKeys(void);

/**
 * Handle function keys F1 ... F4
 *
 * Returns: Number of characters written to dest
 * Parameters: dest: buffer to write string to (minimum sizeof("F00_DN") bytes)
 *                f: Number of function (1..4)
 */
int8_t functionHandler(char * dest, uint8_t f);

#endif
