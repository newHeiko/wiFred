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
#define NUM_AD_SAMPLES 16

/**
 * Tolerance for a new speed to be taken as "same speed"
 */
#define SPEED_TOLERANCE 1

/**
 * Low battery voltage (in millivolts)
 */
#define LOW_BATTERY_VOLTAGE 3500

/**
 * Empty battery voltage (shutdown system in x seconds) (in millivolts)
 */
#define EMPTY_BATTERY_VOLTAGE 3300

/**
 * Initialize A/D converter for free-running conversion mode
 */
void initADC(void);

/**
 * Report if there is a new speed value
 */
bool speedTriggered(void);

/**
 * Returns current speed value read from potentiometer (0..126)
 */
uint8_t getADCSpeed(void);

/**
 * Returns current battery voltage (in millivolts)
 */
uint16_t getBatteryVoltage(void);

#endif
