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

#ifdef LITHIUM_BATTERY
#warning "Compiling for lithium battery version with 8 function keys"
#define KEY_F1      (1ul<<PC0)
#define KEY_F4      (1ul<<PC1)
#define KEY_F7      (1ul<<PC2)
#define KEY_ESTOP   (1ul<<PC3)
#define KEY_F0      (1ul<<(PC0+8))
#define KEY_F2      (1ul<<(PC1+8))
#define KEY_F5      (1ul<<(PC2+8))
#define KEY_F8      (1ul<<(PC3+8))
#define KEY_F3      (1ul<<(PC0+16))
#define KEY_F6      (1ul<<(PC1+16))
#define KEY_SHIFT   (1ul<<(PC3+16))
#define KEY_REVERSE (1ul<<(PC1+24))
#define KEY_FORWARD (1ul<<(PC2+24))
#define KEY_LOCO1   (1ul<<PD7)
#define KEY_LOCO2   (1ul<<PD6)
#define KEY_LOCO3   (1ul<<PD5)
#define KEY_LOCO4   (1ul<<PD4)

#define KEY_ALL     KEY_FORWARD | KEY_REVERSE | KEY_ESTOP | KEY_SHIFT | KEY_F0 | KEY_F1 | KEY_F2 | KEY_F3 | KEY_F4 | KEY_F5 | KEY_F6 | KEY_F7 | KEY_F8 | KEY_LOCO1 | KEY_LOCO2 | KEY_LOCO3 | KEY_LOCO4

#else
#warning "Compiling for newAgeEnclosures version with 6 function keys"
#define KEY_ESTOP   (1ul<<PC0)
#define KEY_F1      (1ul<<PC1)
#define KEY_F4      (1ul<<PC2)
#define KEY_F0      (1ul<<(PC0+8))
#define KEY_F2      (1ul<<(PC1+8))
#define KEY_F5      (1ul<<(PC2+8))
#define KEY_SHIFT   (1ul<<(PC0+16))
#define KEY_F3      (1ul<<(PC1+16))
#define KEY_F6      (1ul<<(PC2+16))
#define KEY_REVERSE (1ul<<(PC1+24))
#define KEY_FORWARD (1ul<<(PC2+24))
#define KEY_LOCO1   (1ul<<PD7)
#define KEY_LOCO2   (1ul<<PD6)
#define KEY_LOCO3   (1ul<<PD5)
#define KEY_LOCO4   (1ul<<PD4)

#define KEY_ALL     KEY_FORWARD | KEY_REVERSE | KEY_ESTOP | KEY_SHIFT | KEY_F0 | KEY_F1 | KEY_F2 | KEY_F3 | KEY_F4 | KEY_F5 | KEY_F6 | KEY_LOCO1 | KEY_LOCO2 | KEY_LOCO3 | KEY_LOCO4

#endif

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
int8_t functionHandler(char * dest, uint8_t f);

#endif
