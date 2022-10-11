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

#include <stdbool.h>
#include <FS.h>
#include <ArduinoJson.h>

#include "config.h"
#include "wifi.h"
#include "locoHandling.h"
#include "throttleHandling.h"

char * throttleName;

/*
 * Read all configuration from EEPROM
 */
void initConfig(void)
{
// initialize to default values
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String tN = "wiFred-" + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);
  throttleName = strdup(tN.c_str());

  locoServer.automatic = true;
  locoServer.name = strdup("undef");
  locoServer.port = 12090;

  for(int i=0; i<4; i++)
  {
    locos[i].address = -1;
    locos[i].direction = DIR_DONTCHANGE;
    locos[i].longAddress = true;
    for(int j=0; j<MAX_FUNCTION + 1; j++)
      {
	      locos[i].functions[j] = THROTTLE;
      }
  }

  centerFunction = CENTER_FUNCTION_IGNORE;
  
  if(!SPIFFS.begin())
  {
    return;
  }
  
  DynamicJsonDocument doc(512);

// read device name from SPIFFS (deprecated)
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

// read general configuration from SPIFFS
  if(File f = SPIFFS.open(FN_CONFIG, "r"))
  {
    if(!deserializeJson(doc, f))
    {
      const char * s = doc[FIELD_NAME_NAME];
      if(s != nullptr)
      {
        free(throttleName);
        throttleName = strdup(s);
      }
      centerFunction = doc[FIELD_CONFIG_CENTERSWITCH] | CENTER_FUNCTION_IGNORE;
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
  for(uint8_t i = 0; true; i++)
  {
    // create new filename
    String filename = String(FN_WIFI_STUB) + i + ".txt";
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
        if(doc.containsKey(FIELD_WIFI_DISABLED))
        {
          newAP.disabled = doc[FIELD_WIFI_DISABLED];
        }
  
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
  for(uint8_t i = 0; i < 4; i++)
  {
    // create new filename
    String filename = String(FN_LOCO_STUB) + (i+1) + ".txt";
    // check if it exists
    if(!SPIFFS.exists(filename))
    {
      continue;
    }

    if(File f = SPIFFS.open(filename, "r"))
    {
      if(!deserializeJson(doc, f))
      {
        locos[i].address = doc[FIELD_LOCO_ADDRESS] | -1;
        if(doc.containsKey(FIELD_LOCO_LONG))
        {
          locos[i].longAddress = doc[FIELD_LOCO_LONG];          
        }
        if(doc.containsKey(FIELD_LOCO_REVERSE))
        {
          locos[i].reverse = doc[FIELD_LOCO_REVERSE];
          if(locos[i].reverse)
          {
            locos[i].direction = DIR_REVERSE;
          }
          else
          {
            locos[i].direction = DIR_NORMAL;
          }
        }
        if(doc.containsKey(FIELD_LOCO_DIRECTION))
        {
          locos[i].direction = doc[FIELD_LOCO_DIRECTION];
        }
        for(uint8_t j = 0; j < MAX_FUNCTION + 1; j++)
        {
          locos[i].functions[j] = (functionInfo) doc[FIELD_LOCO_FUNCTIONS][j].as<int>();
        }
      }
      f.close();
    }
  }
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
  doc[FIELD_CONFIG_CENTERSWITCH] = centerFunction;

  if(File f = SPIFFS.open(FN_CONFIG, "w"))
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

// correct loco address (even before saving)
  if(locos[loco].address > 10239 || locos[loco].address < 0)
  {
    locos[loco].address = -1;
  }
  if(!locos[loco].longAddress && (locos[loco].address > 127 || locos[loco].address < 1))
  {
    locos[loco].address = -1;
  }

  DynamicJsonDocument doc(512);

  // save loco configuration for current loco to SPIFFS
  doc[FIELD_LOCO_ADDRESS] = locos[loco].address;
  doc[FIELD_LOCO_LONG] = locos[loco].longAddress;
  doc[FIELD_LOCO_DIRECTION] = locos[loco].direction;
  doc.createNestedArray(FIELD_LOCO_FUNCTIONS);
  for(uint8_t i = 0; i < MAX_FUNCTION + 1; i++)
  {
    doc[FIELD_LOCO_FUNCTIONS].add((int) (locos[loco].functions[i]) );
  }
  
  String filename = String(FN_LOCO_STUB) + (loco+1) + ".txt";
  if(File f = SPIFFS.open(filename, "w"))
  {
    serializeJson(doc, f);
    f.close();
  }
  
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
    doc[FIELD_WIFI_DISABLED] = it->disabled;

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
