/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2025  Heiko Rosemann
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
#include <stdbool.h>
#include <avr/io.h>
#include "led.h"

volatile ledInfo LEDs[3];

/**
 * Initialize LED output ports
 */
void initLEDs(void)
{
  DDRC |= (1<<PC4);
#ifdef WITH_FLASHLIGHT
  PORTC &= ~(1<<PC4);
#else
  PORTC |= (1<<PC4);
  PORTB |= (1<<PB4) | (1<<PB5);
  DDRB |= (1<<PB4) | (1<<PB5);
#endif
  for(uint8_t led = 0; led < 3; led++)
    {
      LEDs[led].cycleTime = 250;
      LEDs[led].onTime = 0;
    }
}

/**
 * Turn LED on
 *
 * Parameter: i [0..2]: LED to turn on
 */
void setLEDoutput(uint8_t led)
{
#ifndef WITH_FLASHLIGHT
  switch(led)
    {
    case LED_FORWARD:
      PORTB &= ~(1<<PB4);
      break;
    case LED_REVERSE:
      PORTB &= ~(1<<PB5);
      break;
    case LED_STOP:
      PORTC &= ~(1<<PC4);
      break;
    default:
      return;
    }
#else
  if(led < 3)
    {
      LEDs[led].ledStatus = true;
    }
#endif
}
      
/**
 * Turn LED off
 *
 * Parameter: i [0..2]: LED to turn off
 */
void clearLEDoutput(uint8_t led)
{
#ifndef WITH_FLASHLIGHT
  switch(led)
    {
    case LED_FORWARD:
      PORTB |= (1<<PB4);
      break;
    case LED_REVERSE:
      PORTB |= (1<<PB5);
      break;
    case LED_STOP:
      PORTC |= (1<<PC4);
      break;
    default:
      return;
    }
#else
  if(led < 3)
    {
      LEDs[led].ledStatus = false;
    }
#endif
}


