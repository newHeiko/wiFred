/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for reading the potentiometer and calculating the speed value
 *
 * (c) 2018 Heiko Rosemann
 */

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include "analog.h"
#include <avr/interrupt.h>

/**
 * Flag to notify everyone of a speed update
 */
volatile bool newSpeed = false;

/**
 * Current speed value from potentiometer (0...126)
 */
volatile uint8_t currentSpeed = 0;

/**
 * Initialize A/D converter for free-running conversion mode
 */
void initADC(void)
{
  
}

/**
 * Report if there is a new speed value
 */
bool speedTriggered(void)
{
  if(newSpeed)
    {
      newSpeed = false;
      return true;
    }
  else
    {
      return false;
    }
}

/**
 * Returns current speed value read from potentiometer (0..126)
 */
uint8_t getADCSpeed(void)
{
  newSpeed = false;
  return currentSpeed;
}

/**
 * Interrupt handler for AD-converter
 */
ISR(ADC_vect)
{
  #if NUM_AD_SAMPLES > 16
  #warning "Change data type of buffer to accomodate more than 16 samples"
  #endif
  static uint16_t buffer = 0;
  static uint8_t counter;

  buffer += ADC;
  if(counter++ >= NUM_AD_SAMPLES)
    {
      uint8_t temp;
      counter = 0;

      #if NUM_AD_SAMPLES != 16
      #warning "Change divisor so 1023 * NUM_AD_SAMPLES / divisor = 126"
      #endif
      temp = buffer / 129;
      if(temp != currentSpeed)
	{
	  newSpeed = true;
	  currentSpeed = temp;
	}
      buffer = 0;
    }
}
