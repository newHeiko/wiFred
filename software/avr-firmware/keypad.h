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

#include <avr/io.h>

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include "commonKey.h"

/**
 * Function to enable IRQ to wake up from power down mode
 */
void enableWakeup(void);

/**
 * Return current key status
 */
uint32_t getKeyState(uint32_t keyMask);

/**
 * Return new key presses since last call
 */
uint32_t getKeyPresses(uint32_t keyMask);

/**
 * Return new key releases since last call
 */
uint32_t getKeyReleases(uint32_t keyMask);

/**
 * To be called every 2.5ms from ISR - handle key input
 */
void debounceKeys(void);

/**
 * Handle function keys F1 ... F8
 *
 * Returns: Number of characters written to dest
 * Parameters: dest: buffer to write string to (minimum sizeof("F00_DN") bytes)
 *                f: Number of function (1..8)
 */
//int8_t functionHandler(char * dest, uint8_t f);

#endif
