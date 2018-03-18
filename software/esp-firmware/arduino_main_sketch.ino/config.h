#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <EEPROM.h>
#include <stdbool.h>
#include <stdint.h>
#include "wifi.h"

#define SERVER_CHARS 21

#define EEPROM_VALID 3

typedef struct
{
  char name[SERVER_CHARS];
  uint16_t port;
} serverInfo;

#include "clockHandling.h"
#include "locoHandling.h"

#define NAME_CHARS 21
extern char throttleName[NAME_CHARS];


enum eepromAddresses { ID, NAME, WLAN_SSID = NAME + NAME_CHARS, WLAN_KEY = WLAN_SSID + SSID_CHARS,
  CLOCK_ACTIVE = WLAN_KEY + KEY_CHARS, CLOCK_SERVER, CLOCK_PULSE_LENGTH = CLOCK_SERVER + sizeof(serverInfo), CLOCK_MAX_RATE, CLOCK_OFFSET, CLOCK_STARTUP,
  LOCO_ACTIVE = CLOCK_STARTUP + sizeof(clockInfo), LOCO_SERVER, LOCO1 = LOCO_SERVER + sizeof(serverInfo), LOCO2 = LOCO1 + sizeof(locoInfo), LOCO3 = LOCO2 + sizeof(locoInfo), LOCO4 = LOCO3 + sizeof(locoInfo)
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

