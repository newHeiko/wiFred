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
 * Handle function keys F1 ... F8 for lithium-battery version
 * Handle function keys F1 ... F6 for newAgeEnclosures version
 *
 * Returns: Number of characters written to dest
 * Parameters: dest: buffer to write string to (minimum sizeof("F00_DN") bytes)
 *                f: Number of function (1..8) for lithium-battery version
 *                f: Number of function (1..6) for newAgeEnclosures version
 */
int8_t functionHandler(char * dest, uint8_t f)
{
#ifdef LITHIUM_BATTERY
  static uint8_t funcNum[] = {1, 2, 3, 4, 5, 6, 7, 8};
  const uint32_t keyMask[] = {KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8};
#else
  static uint8_t funcNum[] = {1, 2, 3, 4, 5, 6};
  const uint32_t keyMask[] = {KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6};
#endif

  f--;

#ifdef LITHIUM_BATTERY
  if(f > 7)
#else
  if(f > 5)
#endif
    {
      return -1;
    }
  
  if(getKeyPresses(keyMask[f]))
    {
      funcNum[f] = f+1;
      if(getKeyState(KEY_SHIFT))
	{
#ifdef LITHIUM_BATTERY
      funcNum[f] += 8;
#else
      funcNum[f] += 6;
#endif
        }
      return snprintf(dest, sizeof("F00_DN\r\n"), "F%u_DN\r\n", funcNum[f]);
    }
  if(getKeyReleases(keyMask[f]))
    {
      return snprintf(dest, sizeof("F00_UP\r\n"), "F%u_UP\r\n", funcNum[f]);
    }
  return 0;
}	

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
  keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;
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
 * To be called every 2.5ms from ISR - handle key input
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
      DDRB |= (1<<(ct2/2));
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
