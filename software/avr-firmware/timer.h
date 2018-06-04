/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for system base timer, low power timeout and LED output setting
 *
 * (c) 2018 Heiko Rosemann
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

/**
 * Timeout within which a keepalive packet has to be received for the system not to shut off
 */
#define SYSTEM_KEEPALIVE_TIMEOUT 60

/**
 * Countdown for keep alive timeout
 */
extern volatile uint8_t keepaliveCountdownSeconds;

/**
 * Initialize timers
 */
void initTimers(void);

#endif
