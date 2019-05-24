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

enum eLocoState { LOCO_ACTIVATE, LOCO_FUNCTIONS, LOCO_ACTIVE, LOCO_DEACTIVATE, LOCO_INACTIVE };

extern eLocoState locoState[4];

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
extern serverInfo locoServer;

extern char * automaticServer;
extern IPAddress automaticServerIP;

/**
 * Remember the Loco Address plus its prefix (L or S)
 */
extern String locoThrottleID[4];

/**
 * Connect to wiThrottle server
 */
void locoConnect(void);

/**
 * Disconnect from wiThrottle server
 */
void locoDisconnect(void);

/**
 * Initialize connection to wiThrottle server with client ID etc. after receiving the greeting message
 */
void locoRegister(void);

/**
 * Call periodically to check response from wiThrottle server and send new speed/direction info
 */
void locoHandler(void);

/**
 * Transfer new speed value to wiThrottle handling code
 * 
 * Will be sent out by locoHandler if changed
 */
void setSpeed(uint8_t newSpeed);

/**
 * Send new direction value to wiThrottle handling code
 * 
 * Will be sent out to all locos if it is new
 * 
 * Only accepts the direction change if speed is zero
 */
void setReverse(bool newReverse);

/**
 * Retrieve current direction - returns true when reverse
 */
bool getReverse(void);

/**
 * Activate function (only of currently connected and function is throttle controlled)
 */
void setFunction(uint8_t f);

/**
 * Deactivate function (only of currently connected and function is throttle controlled)
 */
void clearFunction(uint8_t f);

/**
 * Set current throttle status to ESTOP
 */
void setESTOP(void);

/**
 * Acquire a new loco for this throttle, including function setting according to function infos
 */
void requestLoco(uint8_t loco);

/**
 * Correctly set functions on newly acquired loco
 */
void requestLocoFunctions(uint8_t loco);

#endif

