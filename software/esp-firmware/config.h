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
 * This file provides functions for handling configuration settings in
 * non-volatile memory.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <EEPROM.h>
#include <stdbool.h>
#include <stdint.h>
#include "wifiHandling.h"
#include "locoHandling.h"

// Filenames and field names on SPIFFS
#define FN_SERVER "/server.txt"
#define FIELD_SERVER_NAME "name"
#define FIELD_SERVER_PORT "port"
#define FIELD_SERVER_AUTOMATIC "automatic"

#define FN_NAME "/name.txt"
#define FIELD_NAME_NAME "name"

#define FN_WIFI_STUB "/wifi"
#define FIELD_WIFI_SSID "ssid"
#define FIELD_WIFI_PSK "key"
#define FIELD_WIFI_DISABLED "disabled"

#define FN_LOCO_STUB "/loco"
#define FIELD_LOCO_ADDRESS "address"
#define FIELD_LOCO_MODE "mode"
#define FIELD_LOCO_LONG "long"
#define FIELD_LOCO_REVERSE "reverse"
#define FIELD_LOCO_DIRECTION "direction"
#define FIELD_LOCO_FUNCTIONS "functions"

#define FN_ANALOG "/calibration.txt"
#define FIELD_POTI_MAX "potiMax"
#define FIELD_POTI_MIN "potiMin"
#define FIELD_BATT_FACTOR "battFactor"

#define FN_CONFIG "/config.txt"
#define FIELD_CONFIG_CENTERSWITCH "centerSwitch"

/**
 * A user-given name for this device
 */
extern char * throttleName;

/**
 * Read all configuration from SPIFFS
 */
void initConfig(void);

/**
 * Save general throttle configuration
 */
void saveGeneralConfig();

/**
 * Save loco server settings
 */
void saveLocoServer();

/**
 * Save loco configuration (also call to save function mappings)
 * 
 * @param loco  Number of loco to save [0..3]
 */
void saveLocoConfig(uint8_t loco);

/**
 * Save WiFi configuration
 */
void saveWiFiConfig();

/**
 * Reformat configuration filesystem
 * Resets everything to factory defaults
 */
void deleteAllConfig();

#endif
