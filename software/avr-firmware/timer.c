/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for system base timer, low power timeout and LED output setting
 *
 * (c) 2018 Heiko Rosemann
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "timer.h"
#include "led.h"

/**
 * Countdown for keep alive timeout
 */
volatile uint8_t keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;

/**
 * Countdown for LED on time
 */
volatile uint8_t ledOntimeCountdown[3];

/**
 * Countdown for LED cycle time
 */
volatile uint8_t ledCycletimeCountdown[3];

/**
 * Initialize timers
 */
void initTimers(void)
{
  TCCR0A = (1<<WGM01);
  OCR0A = F_CPU / 1024 / 100;
  TCCR0B = (1<<CS02) | (1<<CS01) | (1<<CS00);
  TIMSK0 = (1<<TOIE0);
}

ISR(TIMER0_OVF_vect)
{
  static uint8_t secondCountdown;
  if(--secondCountdown == 0)
    {
      if(--keepaliveCountdownSeconds == 0)
	{
	  // shutdown system
	  #warning "Re-enable this after debugging!"
	  /* 
	  sleep_bod_disable();
	  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	  sleep_mode();
	  /* */
	}
      secondCountdown = 100;
    }

  // add in here: keypad debouncing

  for(uint8_t i = 0; i < 3; i++)
    {
      if(ledOntimeCountdown[i] == 0 || --ledOntimeCountdown[i] == 0)
	{
	  DDRD |= (1<<LEDs[i].portPin);
	  ledOntimeCountdown[i] = 1;
	}
      if(--ledCycletimeCountdown[i] == 0)
	{
	  DDRD &= ~(1<<LEDs[i].portPin);
	  ledCycletimeCountdown[i] = LEDs[i].cycleTime;
	  ledOntimeCountdown[i] = LEDs[i].onTime;
	}
      if(ledOntimeCountdown[i] == 0)
	{
	  DDRD |= (1<<LEDs[i].portPin);
	}
    }
}
