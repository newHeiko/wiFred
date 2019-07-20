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
 * This file provides functions and variables for saving the battery voltage.
 */

#ifndef _LOWBAT_H_
#define _LOWBAT_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Number of samples the ADC shall take for averaging
 */
#define NUM_AD_SAMPLES 32
/**
 * Battery voltage for the device to detect low battery status
 * Will change behaviour of device
 */
#define LOW_BATTERY_MILLIVOLTS 2100
/**
 * Battery voltage for the device to shut down (deep sleep, never wakeup)
 */
#define EMPTY_BATTERY_MILLIVOLTS 1800

/**
 * Set to true when the device detects a battery voltage below @LOW_BATTERY_MILLIVOLTS above
 */
extern bool lowBattery;

/**
 * Initialize battery voltage measurement and low battery handling
 */
void lowBatteryInit(void);

/**
 * Periodically check battery voltage and react if falling below the above defined thresholds
 */
void lowBatteryHandler(void);

/**
 * Battery voltage in milliVolt (capped at approx 3V)
 */
extern uint16_t batteryVoltage;

#endif

