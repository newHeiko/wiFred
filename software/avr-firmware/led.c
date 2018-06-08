/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions and data structures for reading and writing LED values
 *
 * (c) 2018 Heiko Rosemann
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

  LEDs[0].cycleTime = 100;
  LEDs[0].onTime = 50;

  LEDs[1].cycleTime = 200;
  LEDs[1].onTime = 50;

  LEDs[2].cycleTime = 100;
  LEDs[2].onTime = 80;

  DDRD = (1<<PD2) | (1<<PD3) | (1<<PD5);
}
