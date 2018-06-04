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
#include <avr/power.h>

#include "analog.h"
#include "uart.h"

int main(void)
{
  // initialize power save settings and system clock prescaler
  power_spi_disable();
  power_twi_disable();
  clock_prescale_set(clock_div_4);
  
  initADC();
  initUART();

  sei();

  while(true)
    {
      ;
    }
}
