/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for reading the potentiometer and calculating the speed value
 *
 * (c) 2018 Heiko Rosemann
 */

#ifndef _ANALOG_H_
#define _ANALOG_H_

#include <stdbool.h>
#include <avr/io.h>
#include <stdint.h>

/**
 * Number of ADC samples to take for averaging
 */
#define NUM_AD_SAMPLES 16

/**
 * Tolerance for a new speed to be taken as "same speed"
 */
#define SPEED_TOLERANCE 1

/**
 * Initialize A/D converter for free-running conversion mode
 */
void initADC(void);

/**
 * Report if there is a new speed value
 */
bool speedTriggered(void);

/**
 * Returns current speed value read from potentiometer (0..126)
 */
uint8_t getADCSpeed(void);

#endif
