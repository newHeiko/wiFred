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
 * This file provides functions for handling the internal clock and driving the external clock.
 */

#ifndef _CLOCK_HANDLING_H_
#define _CLOCK_HANDLING_H_

#include <ESP8266WiFi.h>
#include <stdint.h>
#include <stdbool.h>

// pins on which the clock outputs are located
#define CLOCK1_PIN 16
#define CLOCK2_PIN 14

// maximum value for which times are considered to be the same
#define CLOCK_DELTA 10

extern uint8_t clockPulseLength;
extern uint8_t clockMaxRate;
extern bool clockActive;
extern int8_t clockOffset;

typedef struct
{
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t rate10;
} clockInfo;

#include "config.h"

extern clockInfo ourTime;
extern clockInfo networkTime;
extern clockInfo startupTime;

extern serverInfo clockServer;

void initClock(void);

void clockHandler(void);

#endif

