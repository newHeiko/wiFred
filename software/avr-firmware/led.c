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
 * This file provides functions and data structures for reading and writing LED values
 */

#include <stdint.h>
#include <avr/io.h>
#include "led.h"

volatile ledInfo LEDs[3];

void initLEDs(void)
{
  LEDs[0].portBitmask = (1<<PD2);
  LEDs[1].portBitmask = (1<<PD3);
  LEDs[2].portBitmask = (1<<PD5);

  DDRD = (1<<PD2) | (1<<PD3) | (1<<PD5);
}
