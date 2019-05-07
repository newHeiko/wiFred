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
 * Fuse settings required for this code: Low: 0x7F, High: 0xD9, Extended: 0x07
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

  // check for "correct" EEPROM initialization
  // if not correct, set default, turn on ESP8266 (for programming) and wait for reset
  // this helps during initial flashing of device
  if(vBandgap == UINT16_MAX)
    {
      uint8_t led = LED_STOP;
      LEDs[led].onTime = 50;
      LEDs[LED_STOP].cycleTime = 100;
      LEDs[LED_FORWARD].cycleTime = 100;
      LEDs[LED_REVERSE].cycleTime = 100;
      
      PORTD |= (1<<PD3);
      saveBandgapVoltage(1200);

      while(true)
	{
	  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	  {	    
	    keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;
	  }
	  if(getKeyPresses(KEY_ALL))
	    {
	      LEDs[led].onTime = 0;
	      led++;
	      if(led > 2)
		{
		  led = 0;
		}
	      LEDs[led].onTime = 50;
	    }
	}
    }
  
  while(true)
    {
      uartHandler();

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

	  snprintf(buffer, sizeof("V:1234\r\n"), "V:%04u\r\n", getBatteryVoltage());
	  uartSendData(buffer, sizeof("V:1234\r\n") - 1);
	  
	  if(getBatteryVoltage() < LOW_BATTERY_VOLTAGE)
	    {
	      uartSendData("BLOW\r\n", sizeof("BLOW\r\n") - 1);
	    }
	  else
	    {
	      uartSendData("BOK\r\n", sizeof("BOK\r\n") - 1);
	    }

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
      for(uint8_t f=1; f<9; f++)
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
	    if(getKeyState(KEY_SHIFT))
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
      if(wifiOnline)
	{
	  if(getKeyReleases(KEY_LOCO1) && !getKeyState(KEY_LOCO1))
	    {
	      uartSendData("-L1\r\n", sizeof("-L1\r\n") - 1);
	    }
	  if(getKeyReleases(KEY_LOCO2) && !getKeyState(KEY_LOCO2))
	    {
	      uartSendData("-L2\r\n", sizeof("-L2\r\n") - 1);
	    }
	  if(getKeyReleases(KEY_LOCO3) && !getKeyState(KEY_LOCO3))
	    {
	      uartSendData("-L3\r\n", sizeof("-L3\r\n") - 1);
	    }
	  if(getKeyReleases(KEY_LOCO4) && !getKeyState(KEY_LOCO4))
	    {
	      uartSendData("-L4\r\n", sizeof("-L4\r\n") - 1);
	    }
	  if(getKeyPresses(KEY_LOCO1) && getKeyState(KEY_LOCO1))
	    {
	      uartSendData("+L1\r\n", sizeof("+L1\r\n") - 1);
	    }
	  if(getKeyPresses(KEY_LOCO2) && getKeyState(KEY_LOCO2))
	    {
	      uartSendData("+L2\r\n", sizeof("+L2\r\n") - 1);
	    }
	  if(getKeyPresses(KEY_LOCO3) && getKeyState(KEY_LOCO3))
	    {
	      uartSendData("+L3\r\n", sizeof("+L3\r\n") - 1);
	    }
	  if(getKeyPresses(KEY_LOCO4) && getKeyState(KEY_LOCO4))
	    {
	      uartSendData("+L4\r\n", sizeof("+L4\r\n") - 1);
	    }	  
	}
      
      if(getKeyState(KEY_LOCO1 | KEY_LOCO2 | KEY_LOCO3 | KEY_LOCO4))
	{
	  if(getBatteryVoltage() > LOW_BATTERY_VOLTAGE)
	    {
	      // enable ESP8266 power
	      PORTD |= (1<<PD3);
	      // enable power to speed potentiometer	      
	      PORTC |= (1<<PC5);
	      // enable serial TX
	      UCSR0B |= (1<<TXEN0);
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
	      uartSendData("BEMPTY\r\n", sizeof("BEMPTY\r\n") - 1);
	      _delay_ms(SYSTEM_KEEPALIVE_TIMEOUT * (1000 / 4));
	    }
	     
	}

      // Send message to ESP8266 for power down just before actually powering down
      if(keepaliveCountdownSeconds < SYSTEM_KEEPALIVE_TIMEOUT / 16)
	{
	  uartSendData("PWR_DOWN\r\n", sizeof("PWR_DOWN\r\n") - 1);
	  while(keepaliveCountdownSeconds < SYSTEM_KEEPALIVE_TIMEOUT / 16)
	    ;
	}	  
      
      // show that ESP8266 is not active / battery down
      if(!(PORTD & (1<<PD3)))
	{
	  LEDs[LED_STOP].onTime = 1;
	  LEDs[LED_STOP].cycleTime = 250;
	}
    }
}
