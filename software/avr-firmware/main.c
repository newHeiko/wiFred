/**
 * This file is part of the wiFred wireless throttle project
 *
 * It ties everything together
 *
 * Fuse settings: Low: 0x77, High: 0xD9, Extended: 0xff
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

      if(getKeyPresses(KEY_FORWARD | KEY_REVERSE) || speedTriggered() || speedTimeout == 0)
	{
	  uint8_t speed = getADCSpeed();
	  char buffer[sizeof("S:100:F\r \n")];
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
	  uartSendData(buffer, sizeof("S:100:F\r\n"));
	  speedTimeout = SPEED_INTERVAL;
	}
      
      if(getKeyPresses(KEY_F0))
	{
	  uartSendData("F0_DN\r\n", sizeof("F0_DN\r\n"));
	}      
      if(getKeyReleases(KEY_F0))
	{
	  uartSendData("F0_UP\r\n", sizeof("F0_UP\r\n"));
	}
      for(uint8_t f=1; f<5; f++)
	{
	  char buffer[sizeof("F00_DN\r\n ")];
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
	    uartSendData("ESTOP_DN\r\n", sizeof("ESTOP_DN\r\n"));
	    if(getKeyState(KEY_SHIFT | KEY_SHIFT2))
	      {
		config = true;
		uartSendData("CONF_DN\r\n", sizeof("CONF_DN\r\n"));
	      }
	  }
	if(getKeyReleases(KEY_ESTOP))
	  {
	    uartSendData("ESTOP_UP\r\n", sizeof("ESTOP_UP\r\n"));
	    if(config)
	      {
		uartSendData("CONF_UP\r\n", sizeof("CONF_UP\r\n"));
	      }
	  }
      }	   			   
    }
}