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
#include <util/atomic.h>
#include <util/delay.h>

#include "analog.h"
#include "uart.h"
#include "led.h"
#include "timer.h"
#include "keypad.h"

int main(void)
{
  // this will be set to true for low battery status
  static bool lowBattery = false;

  // initialize power save settings and system clock prescaler
  power_spi_disable();
  power_twi_disable();
  power_timer0_enable();
  clock_prescale_set(clock_div_4);

  // enable pullup resistors and matrix readout
  PORTC = 0x0f;
  PORTD = 0xf0 | (1<<PD2);

  // enable output for enabling ESP8266
  DDRD |= (1<<PD3);

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

	  snprintf(buffer, sizeof("Vxxxx\r\n"), "V:%04u\r\n", getBatteryVoltage());
	  uartSendData(buffer, sizeof("Vxxxx\r\n"));

	  if(lowBattery)
	    {
	      uartSendData("BLOW\r\n", sizeof("BLOW\r\n"));
	    }
	  
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
#ifdef LITHIUM_BATTERY
      for(uint8_t f=1; f<9; f++)
#else
      for(uint8_t f=1; f<7; f++)
#endif
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
	    if(getKeyState(KEY_SHIFT))
	      {
		config = true;
		uartSendData("CONF_DN\r\n", sizeof("CONF_DN\r\n"));
	      }
	    else
	      {
		uartSendData("ESTOP_DN\r\n", sizeof("ESTOP_DN\r\n"));
	      }
	  }
	if(getKeyReleases(KEY_ESTOP))
	  {
	    if(config)
	      {
		uartSendData("CONF_UP\r\n", sizeof("CONF_UP\r\n"));
	      }
	    else
	      {
		uartSendData("ESTOP_UP\r\n", sizeof("ESTOP_UP\r\n"));
	      }	    
	  }
      }
      if(wifiOnline)
	{
	  if(getKeyReleases(KEY_LOCO1))
	    {
	      uartSendData("-L1\r\n", sizeof("-L1\r\n"));
	    }
	  if(getKeyReleases(KEY_LOCO2))
	    {
	      uartSendData("-L2\r\n", sizeof("-L2\r\n"));
	    }
	  if(getKeyReleases(KEY_LOCO3))
	    {
	      uartSendData("-L3\r\n", sizeof("-L3\r\n"));
	    }
	  if(getKeyReleases(KEY_LOCO4))
	    {
	      uartSendData("-L4\r\n", sizeof("-L4\r\n"));
	    }
	  if(getKeyPresses(KEY_LOCO1))
	    {
	      uartSendData("+L1\r\n", sizeof("+L1\r\n"));
	    }
	  if(getKeyPresses(KEY_LOCO2))
	    {
	      uartSendData("+L2\r\n", sizeof("+L2\r\n"));
	    }
	  if(getKeyPresses(KEY_LOCO3))
	    {
	      uartSendData("+L3\r\n", sizeof("+L3\r\n"));
	    }
	  if(getKeyPresses(KEY_LOCO4))
	    {
	      uartSendData("+L4\r\n", sizeof("+L4\r\n"));
	    }
	}
      
      if(getKeyState(KEY_LOCO1 | KEY_LOCO2 | KEY_LOCO3 | KEY_LOCO4))
	{
	  if(getBatteryVoltage() > LOW_BATTERY_VOLTAGE)
	    {
	      lowBattery = false;
	      // enable ESP8266 power
	      PORTD |= (1<<PD3);
	    }
	  else
	    {
	      lowBattery = true;
	    }
	      
	  if(getBatteryVoltage() > EMPTY_BATTERY_VOLTAGE)
	    {
	      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	      {	    
		keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;
	      }
	    }
	  else
	    {
	      uartSendData("BEMPTY\r\n", sizeof("BEMPTY\r\n"));
	      _delay_ms(SYSTEM_KEEPALIVE_TIMEOUT * 1000 / 4);
	    }
	     
	}
    }
}
