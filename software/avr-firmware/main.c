/**
 * This file is part of the wiFred wireless throttle project
 *
 * It ties everything together
 *
 * (c) 2018 Heiko Rosemann
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
  clock_prescale_set(clock_div_4);
  
  initADC();
  initUART();
  initLEDs();
  initTimers();

  sei();

  while(true)
    {
      uartHandler();

      if(getKeyPresses(KEY_FORWARD | KEY_REVERSE) || speedTriggered() || speedTimeout == 0)
	{
	  char buffer[sizeof("S:%03u:F\n")];
	  if(getKeyState(KEY_FORWARD))
	    {
	      snprintf(buffer, sizeof("S:100:F\n"), "S:%03u:F\n", getADCSpeed());
	    }
	  else if(getKeyState(KEY_REVERSE))
	    {
	      snprintf(buffer, sizeof("S:100:R\n"), "S:%03u:R\n", getADCSpeed());
	    }
	  else
	    {
	      snprintf(buffer, sizeof("S:100:E\n"), "S:%03u:E\n", getADCSpeed());
	    }
	  uartSendData(buffer, sizeof("S:%03u:F\n"));
	  speedTimeout = SPEED_INTERVAL;
	}
      
      if(getKeyPresses(KEY_F0))
	{
	  uartSendData("F0_DN\n", sizeof("F0_DN\n"));
	}      
      if(getKeyReleases(KEY_F0))
	{
	  uartSendData("F0_UP\n", sizeof("F0_UP\n"));
	}
      for(uint8_t f=1; f<5; f++)
	{
	  char buffer[sizeof("F00_DN\n")];
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
	    uartSendData("ESTOP_DN\n", sizeof("ESTOP_DN\n"));
	    if(getKeyState(KEY_SHIFT | KEY_SHIFT2))
	      {
		config = true;
		uartSendData("CONF_DN\n", sizeof("CONF_DN\n"));
	      }
	  }
	if(getKeyReleases(KEY_ESTOP))
	  {
	    uartSendData("ESTOP_UP\n", sizeof("ESTOP_UP\n"));
	    if(config)
	      {
		uartSendData("CONF_UP\n", sizeof("CONF_UP\n"));
	      }
	  }
      }	   			   
    }
}
