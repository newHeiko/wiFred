/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2021  Heiko Rosemann
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
 * This file ties everything together to initialize the hardware and
 * form the main loop.
 * 
 * This project was made for ESP12E/ESP12F with Arduino ESP version 2.7.4
 * Board settings: Generic ESP8266 Module, 2, 115200, 80MHz, 26MHz,
 * 4MB (FS:1MB, OTA:~1019kB), DOUT (compatible), 40MHz, no dtr (aka ck), 
 * Disabled, None, v1.4 Higher Bandwidth, Flash, Legacy, Only Sketch, 
 * nonos-sdk 2.2.1+100 (190703), Basic SSL, /dev/ttyUSB0
 */

#include "wifi.h"
#include "config.h"
#include "locoHandling.h"
#include "lowbat.h"
#include "stateMachine.h"
#include "throttleHandling.h"

#define DEBUG

state wiFredState = STATE_STARTUP;
uint32_t stateTimeout = UINT32_MAX;

void setup() {
// put your setup code here, to run once:

  Serial.begin(115200);
  Serial.setTimeout(10);
  initConfig();
  
  #ifdef DEBUG
  Serial.setDebugOutput(true);
  #else
  Serial.setDebugOutput(false);
  #endif
  delay(100);

  initWiFi();

}

void loop() {
  // put your main code here, to run repeatedly:
  handleWiFi();
  handleThrottle();

  // check for empty battery
  // only if not online
  // if online, check will be done in locoHandler
  // (to disconnect before power down)
  if(emptyBattery &&
      wiFredState != STATE_LOCO_ONLINE)
  {
    switchState(STATE_LOWPOWER_WAITING, 100);
  }
  
#ifdef DEBUG
  static uint32_t test = 0;

  if(test < millis())
  {
    test = millis() + 5000;
    Serial.println(ESP.getFreeHeap());
  }
#endif

  switch(wiFredState)
  {
    case STATE_STARTUP:
      setLEDvalues("0/0", "0/0", "100/200");
      initWiFiSTA();
      switchState(STATE_CONNECTING, TOTAL_NETWORK_TIMEOUT_MS);
      break;
      
    case STATE_CONNECTING:
      setLEDvalues("0/0", "0/0", "100/200");
      if(WiFi.status() == WL_CONNECTED)
      {
        initMDNS();
        switchState(STATE_CONNECTED, 60 * 1000);
      }
      else if(millis() > stateTimeout)
      {
        initWiFiAP();
        switchState(STATE_CONFIG_AP);
      }
      break;

    case STATE_CONNECTED:
      setLEDvalues("0/0", "0/0", "25/50");
      if(WiFi.status() != WL_CONNECTED)
      {
        switchState(STATE_STARTUP);
      }
      locoConnect();
      if(millis() > stateTimeout)
      {
        initWiFiConfigSTA();
        switchState(STATE_CONFIG_STATION_WAITING, 120 * 1000);
      }
      break;

    case STATE_LOCO_CONNECTING:
      if(WiFi.status() != WL_CONNECTED)
      {
        switchState(STATE_STARTUP);
      }
      locoRegister();
      if(millis() > stateTimeout)
      {
        initWiFiConfigSTA();
        switchState(STATE_CONFIG_STATION_WAITING, 120 * 1000);
      }
      break;

    case STATE_LOCO_ONLINE:
      if(WiFi.status() != WL_CONNECTED)
      {
        switchState(STATE_STARTUP);
      }
      locoHandler();
      break;
    
    case STATE_CONFIG_STATION_WAITING:
      setLEDvalues("200/200", "200/200", "200/200");
      if(WiFi.status() != WL_CONNECTED)
      {
        switchState(STATE_STARTUP);
      }
      if(millis() > stateTimeout)
      {
        shutdownWiFiConfigSTA();
        switchState(STATE_LOCO_ONLINE);
      }
      break;

    case STATE_CONFIG_STATION:
      setLEDvalues("200/200", "200/200", "200/200");
      if(WiFi.status() != WL_CONNECTED)
      {
        switchState(STATE_STARTUP);
      }
      break;

    case STATE_LOWPOWER_WAITING:
      setLEDvalues("0/0", "0/0", "1/250");
      if(millis() > stateTimeout)
      {
        locoDisconnect();
        shutdownWiFiSTA();
        switchState(STATE_LOWPOWER);
      }
      break;
    
    case STATE_LOWPOWER:
      setLEDvalues("0/0", "0/0", "1/250");
      // shut down ESP
      ESP.deepSleep(0);
      break;
      
    case STATE_CONFIG_AP:
    // no way to get out of here except for restart
      setLEDvalues("0/0", "0/0", "200/200");
      break;
  }
}

void switchState(state newState, uint32_t timeout)
{
  wiFredState = newState;
  if(timeout == UINT32_MAX)
  {
    stateTimeout = timeout;
  }
  else
  {
    stateTimeout = millis() + timeout;
  }
}
