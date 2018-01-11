#ifndef _LOWBAT_H_
#define _LOWBAT_H_

#include <stdbool.h>

/**
 * Number of samples the ADC shall take for averaging
 */
#define NUM_AD_SAMPLES 32
/**
 * Battery voltage for the device to detect low battery status
 * Will change behaviour of device
 */
#define LOW_BATTERY_MILLIVOLTS 1000
/**
 * Battery voltage for the device to shut down (deep sleep, never wakeup)
 */
#define EMPTY_BATTERY_MILLIVOLTS 800

/**
 * Set to true when the device detects a battery voltage below @LOW_BATTERY_MILLIVOLTS above
 */
extern bool lowBattery;

/**
 * Initialize battery voltage measurement and low battery handling
 */
void lowBatteryInit(void);

/**
 * Periodically check battery voltage and react if falling below the above defined thresholds
 */
void lowBatteryHandler(void);

#endif
