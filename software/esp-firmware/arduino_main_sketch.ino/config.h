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
 * This file provides functions for handling configuration settings in
 * non-volatile memory.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <EEPROM.h>
#include <stdbool.h>
#include <stdint.h>
#include "wifi.h"

#define SERVER_CHARS 21

#define EEPROM_VALID 5

typedef struct
{
  char name[SERVER_CHARS];
  uint16_t port;
} serverInfo;

#include "locoHandling.h"

#define NAME_CHARS 21
extern char throttleName[NAME_CHARS];


enum eepromAddresses { ID_LOCOS, NAME, WLAN_SSID = NAME + NAME_CHARS, WLAN_KEY = WLAN_SSID + SSID_CHARS,
  LOCO_SERVER = WLAN_KEY + KEY_CHARS, LOCO1 = LOCO_SERVER + sizeof(serverInfo), LOCO2 = LOCO1 + sizeof(locoInfo), LOCO3 = LOCO2 + sizeof(locoInfo), LOCO4 = LOCO3 + sizeof(locoInfo)
  };

/**
 * Read all configuration from EEPROM
 */
void initConfig(void);

/**
 * Check for slide switch settings to enter configuration mode and
 * handle configuration requests
 */
void configHandler(void);

/**
 * Save clock configuration
 */
void saveClockConfig();

/**
 * Save general throttle configuration
 */
void saveGeneralConfig();

/**
 * Save loco configuration (also call to save function mappings)
 * 
 * @param mainSave  Set to false when only function mappings are new
 */
void saveLocoConfig(bool mainSave = true);

#endif

