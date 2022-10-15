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
 * This file provides functions for key input handling.
 *
 * The code for de-bouncing has originally been developed by Peter Dannegger
 * and can be found i.e. at https://www.avrfreaks.net/projects/efficient-key-debounce
 * This code has been extended to cover more than one input port and to include
 * key release detection.
 */

#include <stdint.h>
#include <stdio.h>
#include <util/atomic.h>
#include "keypad.h"
#include "uart.h"
#include "timer.h"
#include "led.h"

typedef union
{
  uint32_t data;
  uint8_t byte[4];
} keyInfo;

/**
 * Variable to save current key status
 */
volatile keyInfo keyState;

/**
 * Variable to save key press detections
 */
volatile keyInfo keyPress;

/**
 * Variable to save key release detections
 */
volatile keyInfo keyRelease;

/**
 * Handle function keys F1 ... F8
 *
 * Returns: Number of characters written to dest
 * Parameters: dest: buffer to write string to (minimum sizeof("F00_DN") bytes)
 *                f: Number of function (1..8)
 */

/**
 * Function to enable IRQ to wake up from power down mode
 */
void enableWakeup(void)
{
  EIMSK |= (1<<INT0);
}

/**
 * Wakeup from power down mode
 */
ISR(INT0_vect)
{
  EIMSK &= ~(1<<INT0);
  // re-enable pullups
  PORTD |= 0xf0;
  PORTC |= 0x0f;

  // ESP and speed potentiometer will only be reactivated (from main loop) if battery is not empty

  // re-enable ADC
  ADCSRA |= (1<<ADEN);
  // and start first conversion
  ADCSRA |= (1<<ADSC);

  // enough time to measure battery voltage
  keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT / 4;

  initLEDs();
}

/**
 * Return current key status
 */
uint32_t getKeyState(uint32_t keyMask)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    keyMask &= keyState.data;
  }
  return keyMask;
}

/**
 * Return new key presses since last call
 */
uint32_t getKeyPresses(uint32_t keyMask)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    keyMask &= keyPress.data;
    keyPress.data ^= keyMask;
  }
  return keyMask;
}

/**
 * Return new key releases since last call
 */
uint32_t getKeyReleases(uint32_t keyMask)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    keyMask &= keyRelease.data;
    keyRelease.data ^= keyMask;
  }
  return keyMask;
}

/**
 * Will be called every 0.83ms from ISR - handle key input 
 * and LED output
 */
void debounceKeys(void)
{
  static uint8_t ct0[5] = {0xff, 0xff, 0xff, 0xff, 0xff};
  static uint8_t ct1[5] = {0xff, 0xff, 0xff, 0xff, 0xff};
  static uint8_t ct2 = 0;
  uint8_t i;
  
  
  if(ct2 % 2 == 0)
    {
      DDRB &= ~0x0f;
      DDRC &= ~0x0f;
      PORTC |= 0x0f;
      switch(ct2/2)
	{
	case 0:
	  DDRB |= (1<<0);
	  break;  
	case 1:
	  DDRB |= (1<<1);
	  break;  
	case 2:
	  if(LEDs[LED_STOP].ledStatus)
	    {
	      DDRC |= (1<<PC2);
	    }
	  else
	    {
	      PORTC &= ~(1<<PC2);
	    }	  
	  DDRB |= (1<<2);
	  break;  
	case 3:
	  DDRB |= (1<<3);
	  if(LEDs[LED_FORWARD].ledStatus)
	    {
	      DDRC |= (1<<PC0);
	    }
	  else
	    {
	      PORTC &= ~(1<<PC0);
	    }	  
	  if(LEDs[LED_REVERSE].ledStatus)
	    {
	      DDRC |= (1<<PC3);
	    }
	  else
	    {
	      PORTC &= ~(1<<PC3);
	    }	  
	  break;  
	}
      if(ct2 == 0)
	{
	  i = (keyState.byte[0] & 0xf0) ^ (PIND & 0xf0);
	  ct0[4] = ~( ct0[4] & i );
	  ct1[4] = ct0[4] ^ (ct1[4] & i);
	  i &= ct0[4] & ct1[4];
	  keyState.byte[0] ^= i;
	  keyPress.byte[0] |= keyState.byte[0] & i;
	  keyRelease.byte[0] |= ~keyState.byte[0] & i;
	}
    }
  else
    {
      i = (keyState.byte[ct2/2] & 0x0f) ^ ~(PINC | 0xf0);
      ct0[ct2/2] = ~( ct0[ct2/2] & i );
      ct1[ct2/2] = ct0[ct2/2] ^ (ct1[ct2/2] & i);
      i &= ct0[ct2/2] & ct1[ct2/2];
      keyState.byte[ct2/2] ^= i;
      keyPress.byte[ct2/2] |= keyState.byte[ct2/2] & i;
      keyRelease.byte[ct2/2] |= ~keyState.byte[ct2/2] & i;
    }
      
  ct2++;
  if(ct2 >= 8)
    {
      ct2 = 0;
    }
}
