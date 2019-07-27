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
 * This file provides functions for system base timer, low power timeout and LED output setting
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Timeout within which a keepalive packet has to be received for the system not to shut off
 */
#define SYSTEM_KEEPALIVE_TIMEOUT 8

/**
 * Timeout for sending speed and direction data (10ms units) if not changed
 */
#define SPEED_INTERVAL 250

/**
 * Countdown for speed and direction data timeout
 */
extern volatile uint8_t speedTimeout;

/**
 * Countdown for keep alive timeout
 */
extern volatile uint8_t keepaliveCountdownSeconds;

/**
 * Notify timer subsystem of new LED values
 */
extern void newLEDvalues(void);

/**
 * Initialize timers
 */
void initTimers(void);

#endif
