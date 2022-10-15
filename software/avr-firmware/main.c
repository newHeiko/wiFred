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
 * Fuse settings required for this code: Low: 0x7F, High: 0xD9, Extended: 0xff
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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
      newLEDvalues();

      // turn on ESP8266
      PORTD |= (1<<PD3);

      // populate EEPROM with default values
      saveBandgapVoltage(1200);
      vBandgap = 1200;
      saveDefaultADvalues();

      while(true)
	{
	  keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;
	  if(getKeyPresses(KEY_ALL))
	    {
	      LEDs[led].onTime = 0;
	      led++;
	      if(led > 2)
		{
		  led = 0;
		}
	      LEDs[led].onTime = 50;
	      newLEDvalues();
	    }
	}
    }
  
  while(true)
    {
      uartHandler();
      handleADC();

      if(getKeyPresses(KEY_ALL) || getKeyReleases(KEY_ALL) || keyTimeout == 0)
	{
	  char buffer[sizeof("K:\r\n") + 32];
          char * buf = &buffer[2];
          uint32_t keyState = getKeyState(KEY_ALL);

          strcpy(buffer, "K:");

          for(uint32_t mask = (1ul<<31); mask != 0; mask >>= 1)
            {
              if(keyState & mask)
                {
                  *buf++ = '1';
                }
              else
                {
                  *buf++ = '0';
                }
            }

          strcpy(buf, "\r\n");
                               
	  uartSendData(buffer, sizeof("K:\r\n") + 32 - 1);

	  keyTimeout = SPEED_INTERVAL;
	}
      
      if(speedTriggered() || speedTimeout == 0)
	{
	  if(uartSendSpeed(getADCSpeed()))
	    {
	      clearSpeedTrigger();
              speedTimeout = SPEED_INTERVAL;
	    }
	}

      if(voltageTimeout == 0)
        {
	  char buffer[sizeof("V:1234\r\n")];
	  if(getBatteryVoltage() < LOW_BATTERY_VOLTAGE)
	    {
	      uartSendData("BLOW\r\n", sizeof("BLOW\r\n") - 1);
	    }
	  else
	    {
	      uartSendData("BOK\r\n", sizeof("BOK\r\n") - 1);
	    }

	  snprintf(buffer, sizeof("V:1234\r\n"), "V:%04u\r\n", getBatteryVoltage());
	  uartSendData(buffer, sizeof("V:1234\r\n") - 1);
	  
#ifndef REV
#define REV "unknown"
#endif
	  uartSendData("R:"REV"\r\n", sizeof(REV) + 3);
          
          voltageTimeout = SPEED_INTERVAL * 10;
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
	      keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;
	      batteryEmptyCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT / 4;
	    }
	  else if(batteryEmptyCountdownSeconds == 0)
	    {
  	      uartSendData("BEMPTY\r\n", sizeof("BEMPTY\r\n") - 1);
	      _delay_ms(SYSTEM_KEEPALIVE_TIMEOUT * (1000 / 4));
	    }
	     
	}

#ifdef WITH_FLASHLIGHT
      // activate flashlight while SHIFT key is pressed
      if(getBatteryVoltage() > EMPTY_BATTERY_VOLTAGE)
	{
	  if(getKeyState(KEY_SHIFT))
	    {
	      PORTC |= (1<<PC4);
	    }
	  else
	    {
	      PORTC &= ~(1<<PC4);
	    }
	}
#endif

      // Send message to ESP8266 for power down just before actually powering down
      if(keepaliveCountdownSeconds <= SYSTEM_KEEPALIVE_TIMEOUT / 8)
	{
	  uartSendData("PWR_DOWN\r\n", sizeof("PWR_DOWN\r\n") - 1);
	  while(keepaliveCountdownSeconds <= SYSTEM_KEEPALIVE_TIMEOUT / 8)
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
