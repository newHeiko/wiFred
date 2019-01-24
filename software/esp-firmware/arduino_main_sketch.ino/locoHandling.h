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
 * This file provides functions for connecting to a wiThrottle server and
 * communicating with it, including adding and removing locos and controlling
 * them.
 */

#ifndef _LOCO_HANDLING_H_
#define _LOCO_HANDLING_H_

#define MAX_FUNCTION 16

#include <ESP8266WiFi.h>

#include <stdint.h>
#include <stdbool.h>

enum functionInfo { THROTTLE, ALWAYS_ON, ALWAYS_OFF, UNKNOWN = THROTTLE };

enum eLocoState { LOCO_OFFLINE, LOCO_CONNECTED, 
                  LOCO_ACQUIRING, LOCO_ACQUIRING_FUNCTIONS = LOCO_ACQUIRING + 1, 
                  LOCO_ACQUIRE_SINGLE, LOCO_ACQUIRE_SINGLE_FUNCTIONS = LOCO_ACQUIRE_SINGLE + 1, 
                  LOCO_ONLINE };

extern eLocoState locoState;

extern functionInfo globalFunctionStatus[MAX_FUNCTION + 1];;

typedef struct
{
  int16_t address;
  bool longAddress;
  functionInfo functions[MAX_FUNCTION + 1];
  bool reverse;
} locoInfo;

#include "config.h"

extern locoInfo locos[4];
extern bool locoActive;
extern serverInfo locoServer;
extern bool e_allLocosOff;

/**
 * Remember the Loco Address plus its prefix (L or S)
 */
extern String locoThrottleID[4];

#define LOCO1_INPUT 5
#define LOCO2_INPUT 4
#define LOCO3_INPUT 12
#define LOCO4_INPUT 13

extern const uint8_t inputPins[];

void locoInit(void);

void locoHandler(void);

bool getInputState(uint8_t input);

bool getInputChanged(uint8_t input);

/**
 * Acquire a new loco for this throttle, including function setting according to function infos
 * 
 * Will return the same value if needs to be called more than once, loco + 1 if finished
 */
uint8_t requestLoco(uint8_t loco);

#endif

