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
#include <avr/io.h>
#include <avr/eeprom.h>
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
 * Bandgap voltage (in millivolts)
 */
uint16_t vBandgap;

/**
 * EEPROM copy of bandgap voltage
 */
uint16_t ee_vBandgap EEMEM = 1200;

/**
 * Current battery voltage (in millivolts)
 */
uint16_t batteryVoltage = 3600;

/**
 * Flag to signify AD converter has calculated a new speed value
 */
volatile bool newADSpeedValue;

/**
 * Flag to signify AD converter has calculated a new battery voltage value
 */
volatile bool newADVoltageValue;

/**
 * Raw AD buffer
 */
volatile uint16_t ADValue;

/**
 * Check if there is a new AD value and calculate correct output from it if there is
 */
void handleADC(void)
{
  if(newADSpeedValue)
    {      
#if NUM_AD_SAMPLES != 16
#warning "Change divisor so 1023 * NUM_AD_SAMPLES / divisor = 126"
#endif
      uint8_t temp;
      uint16_t buffer;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
	buffer = ADValue;
      }
	
      temp = 126 - (buffer / 129);
      if(temp != currentSpeed)
	{
	  newSpeed = true;
	  currentSpeed = temp;
	}      
      newADSpeedValue = false;
    }
  if(newADVoltageValue)
    {
      uint16_t buffer;
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
	buffer = ADValue;
      }
      
      batteryVoltage = (uint32_t) vBandgap * 1024 * NUM_AD_SAMPLES / 2 / buffer;
      
      // auto-calibrate vBandgap if measurement is larger than maximum voltage possible when charging LiPo cell
      if(batteryVoltage > 4200)
	{
	  vBandgap--;
	  eeprom_write_word(&ee_vBandgap, vBandgap);
	  batteryVoltage = 4200;
	}
      newADVoltageValue = false;
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

  // prepare to enable power to speed potentiometer
  DDRC |= (1<<PC5);
  // will be enabled in main loop

  // read eeprom copy into RAM
  vBandgap = eeprom_read_word(&ee_vBandgap);
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
 * Returns current battery voltage (in millivolts)
 */
uint16_t getBatteryVoltage(void)
{
  uint16_t temp;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    temp = batteryVoltage;
  }
  return temp;
}

/**
 * Saves new bandgap voltage value to EEPROM
 */
void saveBandgapVoltage(uint16_t value)
{
  eeprom_write_word(&ee_vBandgap, value);
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
  static uint8_t counter = 0;
  static bool speed = true;

  if(speed)
    {
      buffer += ADC;
      if(++counter >= NUM_AD_SAMPLES)
	{
	  ADValue = buffer;
	  newADSpeedValue = true;
	  
	  counter = 0;
	  buffer = 0;
	  // switch over to battery voltage measurement mode
	  speed = false;
	  ADMUX = (ADMUX & 0xf0) | 0x0e;
	}
    }
  else
    {
      // ignore first half of samples to allow bandgap voltage to settle
      if(counter >= NUM_AD_SAMPLES / 2)
	{
	  buffer += ADC;
	}
      if(++counter >= NUM_AD_SAMPLES)
	{
	  ADValue = buffer;
	  newADVoltageValue = true;
	  
	  counter = 0;
	  buffer = 0;
	  // switch over to speed measurement mode
	  speed = true;
	  ADMUX = (ADMUX & 0xf0) | 7;
	}
    } 
  
  // Start next AD conversion
  ADCSRA |= (1<<ADSC);
}
