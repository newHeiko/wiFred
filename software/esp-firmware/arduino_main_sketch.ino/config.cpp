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
#include <FS.h>
#include <ArduinoJson.h>

#include "config.h"
#include "wifi.h"
#include "locoHandling.h"

char * throttleName;

/*
 * Read all configuration from EEPROM
 */
void initConfig(void)
{
// initialize to default values
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String tN = "wiFred-" + String(mac[0], 16) + String(mac[5], 16);
  memcpy(throttleName, tN.c_str(), tN.length() + 1);

  locoServer.automatic = true;
  locoServer.name = strdup("undef");
  locoServer.port = 12090;

  for(int i=0; i<4; i++)
  {
    locos[i].address = -1;
    locos[i].reverse = false;
    locos[i].longAddress = true;
    for(int j=0; j<MAX_FUNCTION + 1; j++)
      {
	locos[i].functions[j] = THROTTLE;
      }
  }
  
  if(!SPIFFS.begin())
  {
    return;
  }
  
  DynamicJsonDocument doc(512);

// read device name from SPIFFS
  if(File f = SPIFFS.open(FN_NAME, "r"))
  {
    if(!deserializeJson(doc, f))
    {
      const char * s = doc[FIELD_NAME_NAME];
      if(s != nullptr)
      {
        free(throttleName);
        throttleName = strdup(s);
      }
    }
    f.close();
  }

// read server configuration/information from SPIFFS
  if(File f = SPIFFS.open(FN_SERVER, "r"))
  {
    if(!deserializeJson(doc, f))
    {
      const char * s = doc[FIELD_SERVER_NAME];
      if(s != nullptr)
      {
        free(locoServer.name);
        locoServer.name = strdup(s);
      }
      uint16_t p = doc[FIELD_SERVER_PORT];
      if(p != 0)
      {
        locoServer.port = p;
      }
      if(doc.containsKey(FIELD_SERVER_AUTOMATIC))
      {
        locoServer.automatic = doc[FIELD_SERVER_AUTOMATIC];
      }
    }
    f.close();
  }

// read as many wifi configurations as there are available from SPIFFS  
  String filename;

  for(uint8_t i = 0; true; i++)
  {
    // create new filename
    filename = String(FN_WIFI_STUB) + i + ".txt";
    // check if it exists, exit loop if not
    if(!SPIFFS.exists(filename))
    {
      break;
    }

    if(File f = SPIFFS.open(filename, "r"))
    {
      if(!deserializeJson(doc, f))
      {
        wifiAPEntry newAP;
        newAP.ssid = strdup(doc[FIELD_WIFI_SSID] | "");
        newAP.key = strdup(doc[FIELD_WIFI_PSK] | "");
  
        if(strcmp(newAP.ssid, ""))
        {
          apList.push_back(newAP);
        }
        else
        {
          free(newAP.ssid);
          free(newAP.key);
        }
      }
      f.close();
    }
  }

  // read four loco configurations from SPIFFS if available
#warning "TODO"
  SPIFFS.end();
}

void saveLocoServer()
{
  if(!SPIFFS.begin())
  {
    return;
  }

  DynamicJsonDocument doc(256);

  doc[FIELD_SERVER_NAME] = locoServer.name;
  doc[FIELD_SERVER_PORT] = locoServer.port;
  doc[FIELD_SERVER_AUTOMATIC] = locoServer.automatic;

  if(File f = SPIFFS.open(FN_SERVER, "w"))
  {
    serializeJson(doc, f);
    f.close();
  }

  SPIFFS.end();
}


void saveGeneralConfig(void)
{
  if(!SPIFFS.begin())
  {
    return;
  }

  DynamicJsonDocument doc(256);

  doc[FIELD_NAME_NAME] = throttleName;

  if(File f = SPIFFS.open(FN_NAME, "w"))
  {
    serializeJson(doc, f);
    f.close();
  }

  SPIFFS.end();
}

void saveLocoConfig(uint8_t loco)
{
  if(loco >= 4 || !SPIFFS.begin())
  {
    return;
  }

  DynamicJsonDocument doc(512);

  // save loco configuration for current loco to SPIFFS

  SPIFFS.end();
}

void saveWiFiConfig()
{
  if(!SPIFFS.begin())
  {
    return;
  }
  
  DynamicJsonDocument doc(256);

  String filename;

  for(uint8_t i = 0; ; i++)
  {
    // create new filename
    filename = String(FN_WIFI_STUB) + i + ".txt";
    // check if it exists, exit loop if not
    if(!SPIFFS.exists(filename))
    {
      break;
    }
    SPIFFS.remove(filename);
  }

  uint8_t i = 0;
  for(std::vector<wifiAPEntry>::iterator it = apList.begin(); it != apList.end(); it++, i++)
  {
    // create new filename
    filename = String(FN_WIFI_STUB) + i + ".txt";

    doc[FIELD_WIFI_SSID] = it->ssid;
    doc[FIELD_WIFI_PSK] = it->key;

    if(File f = SPIFFS.open(filename, "w"))
    {
      serializeJson(doc, f);
      f.close();
    }
    doc.clear();
  }

  SPIFFS.end();
}

void deleteAllConfig()
{
  SPIFFS.format();
}
