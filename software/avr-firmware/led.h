/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions and data structures for reading and writing LED values
 *
 * (c) 2018 Heiko Rosemann
 */

#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>

typedef struct
{
  uint8_t onTime;
  uint8_t cycleTime;
} ledInfo;

ledInfo LEDs[3];

void initLEDs(void);

#endif
