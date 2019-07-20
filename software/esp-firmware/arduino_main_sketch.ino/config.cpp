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

#include <stdbool.h>

#include "config.h"
#include "wifi.h"
#include "locoHandling.h"

bool wifiSaved = false;
bool locoSaved = false;

char throttleName[NAME_CHARS];

void eepromReadBlock(uint8_t * dest, const uint16_t src, size_t length)
{
  uint16_t local_src;
  for(local_src = src; local_src < src + length; dest++, local_src++)
  {
    *dest = EEPROM.read(local_src);
  }
}

void eepromWriteBlock(const uint16_t dest, uint8_t * src, size_t length)
{
  uint16_t local_dest;
  for(local_dest = dest; local_dest < dest + length; local_dest++, src++)
  {
    EEPROM.write(local_dest, *src);
  }
}

/*
 * Read all configuration from EEPROM
 */
void initConfig(void)
{
  EEPROM.begin(512);

  // check for valid EEPROM data
  uint8_t idLoco = EEPROM.read(ID_LOCOS);

// initialize to default values
  memcpy(wlan.ssid, "undef", sizeof("undef"));
  memcpy(wlan.key, "undef", sizeof("undef"));

  uint8_t mac[6];
  WiFi.macAddress(mac);
  String tN = "wiFred-" + String(mac[0], 16) + String(mac[5], 16);
  memcpy(throttleName, tN.c_str(), tN.length() + 1);
  memcpy(locoServer.name, "undef", sizeof("undef"));
  locoServer.port = 12090;

  for(int i=0; i<4; i++)
  {
    locos[i].address = -1;
    locos[i].reverse = false;
    locos[i].longAddress = true;
  }
  
  if(idLoco == EEPROM_VALID) // general EEPROM data is valid
  {
    eepromReadBlock((uint8_t *) wlan.ssid, WLAN_SSID, sizeof(wlan.ssid)/sizeof(wlan.ssid[0]));
    wlan.ssid[sizeof(wlan.ssid)/sizeof(wlan.ssid[0]) - 1] = '\0';
    eepromReadBlock((uint8_t *) wlan.key, WLAN_KEY, sizeof(wlan.key)/sizeof(wlan.key[0]));
    wlan.key[sizeof(wlan.key)/sizeof(wlan.key[0]) - 1] = '\0';
    eepromReadBlock((uint8_t *) throttleName, NAME, sizeof(throttleName)/sizeof(throttleName[0]));
    throttleName[sizeof(throttleName)/sizeof(throttleName[0]) - 1] = '\0';
    wifiSaved = true;
    eepromReadBlock((uint8_t *) &locoServer, LOCO_SERVER, sizeof(locoServer)); 
    eepromReadBlock((uint8_t *) &(locos[0]), LOCO1, sizeof(locos[0]));
    eepromReadBlock((uint8_t *) &(locos[1]), LOCO2, sizeof(locos[1]));
    eepromReadBlock((uint8_t *) &(locos[2]), LOCO3, sizeof(locos[2]));
    eepromReadBlock((uint8_t *) &(locos[3]), LOCO4, sizeof(locos[3]));
    locoSaved = true;
  }
}

void saveGeneralConfig()
{
  eepromWriteBlock(WLAN_SSID, (uint8_t *) wlan.ssid, sizeof(wlan.ssid)/sizeof(wlan.ssid[0]));
  eepromWriteBlock(WLAN_KEY, (uint8_t *) wlan.key, sizeof(wlan.key)/sizeof(wlan.key[0]));
  eepromWriteBlock(NAME, (uint8_t *) throttleName, sizeof(throttleName)/sizeof(throttleName[0]));

  wifiSaved = true;
  if(locoSaved)
  {
    EEPROM.write(ID_LOCOS, EEPROM_VALID);
  }
  EEPROM.commit();
}

void saveLocoConfig(bool mainSave)
{
  eepromWriteBlock(LOCO_SERVER, (uint8_t *) &locoServer, sizeof(locoServer)); 
  eepromWriteBlock(LOCO1, (uint8_t *) &(locos[0]), sizeof(locos[0]));
  eepromWriteBlock(LOCO2, (uint8_t *) &(locos[1]), sizeof(locos[1]));
  eepromWriteBlock(LOCO3, (uint8_t *) &(locos[2]), sizeof(locos[2]));
  eepromWriteBlock(LOCO4, (uint8_t *) &(locos[3]), sizeof(locos[3]));

  if(mainSave)
  {
    locoSaved = true;
    if(wifiSaved)
    {
      EEPROM.write(ID_LOCOS, EEPROM_VALID);
    }
  }
  EEPROM.commit();
}
