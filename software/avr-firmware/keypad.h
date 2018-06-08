/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for key input handling
 *
 * (c) 2018 Heiko Rosemann
 */

#include <stdint.h>

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#define KEY_FORWARD (1<<PB0)
#define KEY_REVERSE (1<<PB1)
#define KEY_ESTOP   (1<<PB3)
#define KEY_SHIFT   (1<<PB4)
#define KEY_SHIFT2  (1<<PB5)
#define KEY_F0      (1<<(PC0+8))
#define KEY_F1      (1<<(PC1+8))
#define KEY_F2      (1<<(PC2+8))
#define KEY_F3      (1<<(PC3+8))
#define KEY_F4      (1<<(PC4+8))

#define KEY_ALL     KEY_FORWARD | KEY_REVERSE | KEY_ESTOP | KEY_SHIFT | KEY_SHIFT2 | KEY_F0 | KEY_F1 | KEY_F2 | KEY_F3 | KEY_F4


/**
 * Return current key status
 */
uint16_t getKeyState(uint16_t keyMask);

/**
 * Return new key presses since last call
 */
uint16_t getKeyPresses(uint16_t keyMask);

/**
 * To be called every 10ms from ISR - handle key input
 */
void debounceKeys(void);

#endif
