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

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "timer.h"
#include "led.h"
#include "keypad.h"

/**
 * Countdown for keep alive timeout
 */
volatile uint8_t keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;

/**
 * Countdown for speed and direction data timeout
 */
volatile uint8_t speedTimeout = SPEED_INTERVAL;

/**
 * If this flag is set, all LED counters start at zero again
 */
volatile bool newLED = true;

/**
 * Notify timer subsystem of new LED values
 */
void newLEDvalues(void)
{
  newLED = true;
}

/**
 * Initialize timers
 */
void initTimers(void)
{
  TCCR0A = (1<<WGM01);
  OCR0A = F_CPU / 1024 / 100;
  OCR0B = OCR0A / 4;
  TCCR0B = (1<<CS02) | (1<<CS00);
  TIMSK0 = (1<<OCIE0A) | (1<<OCIE0B);
}

ISR(TIMER0_COMPB_vect)
{
  if(OCR0B >= OCR0A)
    {
      OCR0B = 0;
    }
  OCR0B += OCR0A / 4;

  debounceKeys();
}

ISR(TIMER0_COMPA_vect)
{
  static uint8_t secondCountdown = 100;
  if(--secondCountdown == 0)
    {
      if(--keepaliveCountdownSeconds == 0)
	{
	  // shutdown system
	  // disable LED outputs
	  clearLEDoutput(0);
	  clearLEDoutput(1);
	  clearLEDoutput(2);
	  // disable unneeded pullups to save power
	  PORTD &= ~(0xf0);
	  PORTC &= ~(0x0f);
	  // enable wakeup method through INT0 IRQ
	  enableWakeup();
	  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	  sleep_enable();
	  sleep_bod_disable();
	  sei();
	  sleep_cpu();
	  sleep_disable();
	  /* */
	}
      secondCountdown = 100;
    }

  if(speedTimeout > 0)
    {
      speedTimeout--;
    }
  
  static uint8_t ledOntimeCountdown[3];
  static uint8_t ledCycletimeCountdown[3];

  if(newLED)
    {
      for(uint8_t i = 0; i < 3; i++)
	{
	  ledOntimeCountdown[i] = 0;
	  ledCycletimeCountdown[i] = 0;
	}
      newLED = false;
    }
  
  for(uint8_t i = 0; i < 3; i++)
    {
      if(ledOntimeCountdown[i] == 0 || --ledOntimeCountdown[i] == 0)
	{
	  clearLEDoutput(i);
	  ledOntimeCountdown[i] = 1;
	}
      if(ledCycletimeCountdown[i] == 0 || --ledCycletimeCountdown[i] == 0)
	{
	  ledCycletimeCountdown[i] = LEDs[i].cycleTime;
	  ledOntimeCountdown[i] = LEDs[i].onTime;
	  if(ledOntimeCountdown[i] != 0)
	    {
	      setLEDoutput(i);
	    }
	}
    }
}
