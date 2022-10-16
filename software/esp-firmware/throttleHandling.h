/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2022 Heiko Rosemann
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
 * This file handles the connection to the AVR handling keys, direction switch 
 * and speed input and also forms proper wiThrottle commands from them to be
 * sent to the server via the functions in locoHandling.*
 */

// defines to match key mask values to AVR
#define PC0 0
#define PC1 1
#define PC2 2
#define PC3 3
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7

#include "commonKey.h"

#ifndef _THROTTLE_HANDLING_H_
#define _THROTTLE_HANDLING_H_

#define CENTER_FUNCTION_ZEROSPEED -1
#define CENTER_FUNCTION_IGNORE -2

#define CENTER_FUNCTION_ESTOP_TIMEOUT 500

/**
 * Send LED settings to AVR - Strings are of the shape "20/100" meaning 20*10ms on time and 100*10ms total cycle time
 */
void setLEDvalues(String led1, String led2, String led3);

/**
 * Array containing all valid key masks
 */
extern const uint32_t keys[];

/**
 * Get state of input buttons
 * 
 * @param the keymask to query
 * @return true if input button is pressed (pin value is low)
 */
bool getInputState(uint32_t key);

/**
 * Define behavior of center-off-switch
 * 
 * 0 or higher: Function to set when switch at center position
 * -1: Set speed to zero
 * -2: Ignore
 */
extern int centerFunction;

/**
 * Status of direction switch
 * 
 * true if in center position
 */
extern bool centerPosition;

/**
 * Set red LED blinking numbers
 * 
 * @param number Blink red LED this many times before resuming normal LED patterns
 */
void setLEDblink(unsigned int number);

/**
 * Change LED settings
 */
void setLEDvalues(String ledFwd, String ledRev, String ledStop);

/**
 * Periodically check serial port for new information from the AVR
 */
void handleThrottle(void);

/**
 * String keeping the AVR firmware revision
 */
extern char * avrRevision;

/**
 * Allow direction change even if speed > 0
 */
bool allowDirectionChange();

/**
 * Block direction change even if speed == 0
 */
bool blockDirectionChange();

/**
 * Show battery voltage through LEDs
 */
void showVoltage(void);

/**
 * Show battery voltage through LEDs only if no loco switch is active
 * Show LED values otherwise
 */
void showVoltageIfOff(String ledFwd, String ledRev, String ledStop);

#endif
