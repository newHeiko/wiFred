/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for key input handling
 *
 * (c) 2018 Heiko Rosemann
 */

#include <stdint.h>
#include <stdio.h>
#include <util/atomic.h>
#include "keypad.h"
#include "uart.h"

typedef union
{
  uint16_t data;
  uint8_t byte[2];
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
 * Handle function keys F1 ... F4
 *
 * Returns: Number of characters written to dest
 * Parameters: dest: buffer to write string to (minimum sizeof("F00_DN") bytes)
 *                f: Number of function (1..4)
 */
int8_t functionHandler(char * dest, uint8_t f)
{
  static uint8_t funcNum[] = {1, 2, 3, 4};
  const uint16_t keyMask[] = {KEY_F1, KEY_F2, KEY_F3, KEY_F4};

  f--;

  if(f > 3)
    {
      return -1;
    }
  
  if(getKeyPresses(keyMask[f]))
    {
      funcNum[f] = f+1;
      if(getKeyState(KEY_SHIFT))
	{
	  funcNum[f] += 4;
	}
      if(getKeyState(KEY_SHIFT2))
	{
	  funcNum[f] += 8;
	}
      return snprintf(dest, sizeof("F00_DN\r\n"), "F%u_DN\r\n", funcNum[f]) + 1;
    }
  if(getKeyReleases(keyMask[f]))
    {
      return snprintf(dest, sizeof("F00_UP\r\n"), "F%u_UP\r\n", funcNum[f]) + 1;
    }
  return 0;
}	

/**
 * Return current key status
 */
uint16_t getKeyState(uint16_t keyMask)
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
uint16_t getKeyPresses(uint16_t keyMask)
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
uint16_t getKeyReleases(uint16_t keyMask)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    keyMask &= keyRelease.data;
    keyRelease.data ^= keyMask;
  }
  return keyMask;
}

/**
 * To be called every 10ms from ISR - handle key input
 */
void debounceKeys(void)
{
  static uint8_t ctB0 = 0xff, ctC0 = 0xff, ctB1 = 0xff, ctC1 = 0xff;
  uint8_t i;

  i = keyState.byte[0] ^ ~PINB;
  ctB0 = ~( ctB0 & i );
  ctB1 = ctB0 ^ (ctB1 & i);
  i &= ctB0 & ctB1;
  keyState.byte[0] ^= i;
  keyPress.byte[0] |= keyState.byte[0] & i;
  keyRelease.byte[0] |= ~keyState.byte[0] & i;

  i = keyState.byte[1] ^ ~PINC;
  ctC0 = ~( ctC0 & i );
  ctC1 = ctC0 ^ (ctC1 & i);
  i &= ctC0 & ctC1;
  keyState.byte[1] ^= i;
  keyPress.byte[1] |= keyState.byte[1] & i;
  keyRelease.byte[1] |= ~keyState.byte[1] & i;
}
