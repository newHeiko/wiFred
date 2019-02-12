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
 * This file provides functions for measuring the battery voltage and shutting
 * down the system if the voltage is too low.
 * 
 * A 100kOhm/47kOhm voltage divider at the ADC input pin is required for proper
 * voltage readings.
 */

#ifndef _LOWBAT_H_
#define _LOWBAT_H_

#include <stdbool.h>

/**
 * Set to true when the device receives a BLOW message, false when receiving a BOK message
 */
extern bool lowBattery;

/**
 * Set to true when the device receives a BEMPTY message
 */
extern bool emptyBattery;

/**
 * Battery voltage in milliVolt
 */
extern uint16_t batteryVoltage;

#endif

