/**
 * This file is part of the wiFred wireless throttle project
 *
 * It ties everything together
 *
 * (c) 2018 Heiko Rosemann
 */

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "analog.h"

int main(void)
{
  initADC();

  // initialize power save settings
  PRR = (1<<PRTWI) | (1<<PRSPI);
  
  while(true)
    {
      ;
    }
}
