#ifndef _CLOCK_HANDLING_H_
#define _CLOCK_HANDLING_H_

#include <ESP8266WiFi.h>
#include <stdint.h>
#include <stdbool.h>

// pins on which the clock outputs are located
#define CLOCK1_PIN 16
#define CLOCK2_PIN 14

// maximum value for which times are considered to be the same
#define CLOCK_DELTA 10

extern uint8_t clockPulseLength;
extern uint8_t clockMaxRate;
extern bool clockActive;
extern int8_t clockOffset;

typedef struct
{
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t rate10;
} clockInfo;

#include "config.h"

extern clockInfo ourTime;
extern clockInfo networkTime;
extern clockInfo startupTime;

extern serverInfo clockServer;

void initClock(void);

void clockHandler(void);

#endif

