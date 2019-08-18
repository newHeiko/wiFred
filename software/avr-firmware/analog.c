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
 * This file provides functions for reading the potentiometer and calculating the speed value
 */

#include <stdbool.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "analog.h"

/**
 * Flag to notify everyone of a speed update
 */
bool newSpeed = false;

/**
 * Current speed value from potentiometer (0...126)
 */
uint8_t currentSpeed = 0;

/**
 * Lowest AD value ever received
 */
uint16_t adLowest;

/**
 * EEPROM copy of lowest AD value
 */
uint16_t ee_adLowest EEMEM = 1024ul * NUM_AD_SAMPLES / 2;

/**
 * Highest AD value ever received
 */
uint16_t adHighest;

/**
 * EEPROM copy of highest AD value
 */
uint16_t ee_adHighest EEMEM = 1024ul * NUM_AD_SAMPLES / 2;

/**
 * Flag to signify AD converter has calculated a new speed value
 */
volatile bool newADSpeedValue;

/**
 * Raw AD buffer
 */
volatile uint16_t ADValue;

/**
 * Table to calculate correct speed value from AD result
 */
const uint8_t speedTable[] = { 0, 1, 2, 4, 5, 7, 9, 11, 13, 16, 18, 21, 24, 27, 30, 33, 36, 40,
			       44, 48, 52, 56, 61, 65, 70, 75, 80, 85, 90, 96, 102, 108, 114, 120, 126 };

/**
 * Check if there is a new AD value and calculate correct output from it if there is
 */
void handleADC(void)
{
  if(newADSpeedValue)
    {      
      static uint16_t oldADValue = 1024ul * NUM_AD_SAMPLES / 2;
      uint8_t temp;
      uint16_t buffer;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
	buffer = ADValue;
      }

      // check for larger voltage range if speed potentiometer enabled
      if(buffer < adLowest)
	{
	  adLowest = buffer;
	  eeprom_write_word(&ee_adLowest, adLowest);
	}

      if(buffer > adHighest)
	{
	  adHighest = buffer;
	  eeprom_write_word(&ee_adHighest, adHighest);
	}

      // calculate tolerance
      uint16_t tolerance = (adHighest - adLowest) / NUM_AD_VALUES;

      // subtract lower border
      buffer -= adLowest;

      // check for change large enough to warrant new speed calculation
      if(buffer < tolerance || buffer > adHighest - tolerance || buffer + tolerance < oldADValue || buffer > oldADValue + tolerance)
	{
	  oldADValue = buffer;
      
	  temp = buffer * 127ul / (adHighest - adLowest);
	  if(temp >= 127)
	    {
	      temp = 126;
	    }
	  temp = 126 - temp;
	  if(temp != currentSpeed)
	    {
	      newSpeed = true;
	      currentSpeed = temp;
	    }
	}
      newADSpeedValue = false;
    }
}

/**
 * Initialize A/D converter for single run conversion mode
 * and start first conversion
 */
void initADC(void)
{
  ADMUX = (1<<REFS0) | 7;
  ADCSRA = (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1);
  ADCSRB = 0;
  ADCSRA |= (1<<ADSC);

  // read highest and lowest AD values into RAM
  adLowest = eeprom_read_word(&ee_adLowest);
  adHighest = eeprom_read_word(&ee_adHighest);

  // check for "correct" EEPROM initialization
  // if not correct, set default and wait for reset
  // this helps during initial flashing of device
  if(adLowest == UINT16_MAX)
    {
      saveDefaultADvalues();

      // read highest and lowest AD values into RAM
      adLowest = eeprom_read_word(&ee_adLowest);
      adHighest = eeprom_read_word(&ee_adHighest);
    }
}

/**
 * Report if there is a new speed value
 */
bool speedTriggered(void)
{
  return newSpeed;
}

/**
 * Clear "new speed value" trigger
 */
void clearSpeedTrigger(void)
{
  newSpeed = false;
}

/**
 * Returns current speed value read from potentiometer (0..126)
 */
uint8_t getADCSpeed(void)
{
  return currentSpeed;
}

/**
 * Saves default lowest/highest AD values to EEPROM
 */
void saveDefaultADvalues(void)
{
  eeprom_write_word(&ee_adLowest, 1024ul * NUM_AD_SAMPLES / 2);
  eeprom_write_word(&ee_adHighest, 1024ul * NUM_AD_SAMPLES / 2);
}

/**
 * Interrupt handler for AD-converter
 */
ISR(ADC_vect)
{
  #if NUM_AD_SAMPLES > 64
  #warning "Change data type of buffer to accomodate more than 64 samples"
  #endif
  static uint16_t buffer = 0;
  static uint8_t counter = 0;

  buffer += ADC;

  if(++counter >= NUM_AD_SAMPLES)
    {
      ADValue = buffer;
      newADSpeedValue = true;
      
      counter = 0;
      buffer = 0;
    }
  // Start next AD conversion
  ADCSRA |= (1<<ADSC);
}
