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
 * This file provides functions for reading the potentiometer and calculating the speed value
 */

#ifndef _ANALOG_H_
#define _ANALOG_H_

#include <stdbool.h>
#include <avr/io.h>
#include <stdint.h>

/**
 * Number of ADC samples to take for averaging
 */
#define NUM_AD_SAMPLES 32

/**
 * Number of discrete ADC values to consider
 *
 * New value will only count as new value if it does not fall into same bin as last
 * Bin size is calculated as (adHighest - adLowest) / AD_NUM_VALUES
 */
#define NUM_AD_VALUES 75

/**
 * Check if there is a new AD value and calculate correct output from it if there is
 */
void handleADC(void);

/**
 * Initialize A/D converter for single run conversion mode
 * and start first conversion
 */
void initADC(void);

/**
 * Report if there is a new speed value
 */
bool speedTriggered(void);

/**
 * Clear "new speed value" trigger
 */
void clearSpeedTrigger(void);

/**
 * Returns current speed value read from potentiometer (0..126)
 */
uint8_t getADCSpeed(void);

/**
 * Saves default lowest/highest AD values to EEPROM
 */
void saveDefaultADvalues(void);

#endif
