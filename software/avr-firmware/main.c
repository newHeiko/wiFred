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
 * This file ties everything together to initialize the hardware and
 * form the main loop.
 *
 * Fuse settings required for this code: Low: 0x77, High: 0xD9, Extended: 0x07
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#include "analog.h"
#include "uart.h"
#include "led.h"
#include "timer.h"
#include "keypad.h"

int main(void)
{
  // initialize power save settings and system clock prescaler
  power_spi_disable();
  power_twi_disable();
  power_timer0_enable();
  clock_prescale_set(clock_div_4);

  // enable pullup resistors
  PORTB = KEY_FORWARD | KEY_REVERSE | KEY_ESTOP | KEY_SHIFT | KEY_SHIFT2;
  PORTC = 0x1f;

  initADC();
  initUART();
  initLEDs();
  initTimers();

  sei();
  
  while(true)
    {
      uartHandler();
      handleADC();

      if(getKeyPresses(KEY_FORWARD | KEY_REVERSE) || speedTimeout == 0)
	{
	  uint8_t speed = getADCSpeed();
	  char buffer[sizeof("S:100:F\r\n")];
	  if(getKeyState(KEY_FORWARD))
	    {
	      snprintf(buffer, sizeof("S:100:F\r\n"), "S:%03u:F\r\n", speed);
	    }
	  else if(getKeyState(KEY_REVERSE))
	    {
	      snprintf(buffer, sizeof("S:100:R\r\n"), "S:%03u:R\r\n", speed);
	    }
	  else
	    {
	      snprintf(buffer, sizeof("S:100:E\r\n"), "S:%03u:E\r\n", speed);
	    }
	  uartSendData(buffer, sizeof("S:100:F\r\n") - 1);

#ifndef REV
#define REV "unknown"
#endif
	  uartSendData("R"REV"\r\n", sizeof(REV) + 2);

	  speedTimeout = SPEED_INTERVAL;
	}
      
      if(speedTriggered())
	{
	  if(uartSendSpeed(getADCSpeed()))
	    {
	      clearSpeedTrigger();
	    }
	}
      
      if(getKeyPresses(KEY_F0))
	{
	  uartSendData("F0_DN\r\n", sizeof("F0_DN\r\n") - 1);
	}      
      if(getKeyReleases(KEY_F0))
	{
	  uartSendData("F0_UP\r\n", sizeof("F0_UP\r\n") - 1);
	}
      for(uint8_t f=1; f<5; f++)
	{
	  char buffer[sizeof("F00_DN\r\n")];
	  int8_t ret = functionHandler(buffer, f);
	  if(ret > 0)
	    {
	      uartSendData(buffer, ret);
	    }
	}
      {
	static bool config = false;
	if(getKeyPresses(KEY_ESTOP))
	  {
	    config = false;
	    if(getKeyState(KEY_SHIFT | KEY_SHIFT2))
	      {
		config = true;
		uartSendData("CONF_DN\r\n", sizeof("CONF_DN\r\n") - 1);
	      }
	    else
	      {
		uartSendData("ESTOP_DN\r\n", sizeof("ESTOP_DN\r\n") - 1);
	      }
	  }
	if(getKeyReleases(KEY_ESTOP))
	  {
	    if(config)
	      {
		uartSendData("CONF_UP\r\n", sizeof("CONF_UP\r\n") - 1);
	      }
	    else
	      {
		uartSendData("ESTOP_UP\r\n", sizeof("ESTOP_UP\r\n") - 1);
	      }	    
	  }
      }	  
    }
}
