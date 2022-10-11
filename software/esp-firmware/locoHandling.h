/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2022 Heiko Rosemann
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

/** 
 * Don't send speed commands more often than this (milliseconds)
 */
#define SPEED_HOLDOFF_PERIOD 150

/**
 * Put wiFred to sleep if not in use for this long time
 */
#define NO_ACTIVITY_TIMEOUT (1000L * 60 * 60 * 3) // 3 hours

#include <WiFi.h>

#include <stdint.h>
#include <stdbool.h>

#include "config.h"

enum functionInfo { THROTTLE, THROTTLE_MOMENTARY, THROTTLE_LOCKING, THROTTLE_SINGLE, ALWAYS_ON, ALWAYS_OFF, IGNORE, UNKNOWN = THROTTLE };
enum eDirection { DIR_NORMAL, DIR_REVERSE, DIR_DONTCHANGE };
enum eLocoState { LOCO_ACTIVATE, LOCO_FUNCTIONS, LOCO_LEAVE_FUNCTIONS, LOCO_ACTIVE, LOCO_DEACTIVATE, LOCO_INACTIVE };

extern eLocoState locoState[4];

extern functionInfo globalFunctionStatus[MAX_FUNCTION + 1];;

typedef struct
{
  bool automatic;
  char * name;
  uint16_t port;
} serverInfo;

typedef struct
{
  int16_t address;
  bool longAddress;
  functionInfo functions[MAX_FUNCTION + 1];
  eDirection direction;
  bool reverse;
} locoInfo;

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
 * Wait for timeout in greeting message
 * 
 * @returns true if timeout received
 */
bool timeoutReceived(void);

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
 * Set correct function and direction settings on newly acquired loco
 */
void setLocoFunctions(uint8_t loco);

/**
 * Read current function and direction settings on newly acquired loco
 */
void getLocoFunctions(uint8_t loco);

/**
 * Are there any active locos left?
 * 
 * @returns true if all locos have been deactivated
 */
bool allLocosInactive(void);

/**
 * Is this the only active loco?
 * 
 * @returns true if all other locos are inactive
 */
bool isOnlyLoco(uint8_t loco);

#endif
