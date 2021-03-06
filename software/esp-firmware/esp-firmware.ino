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
 * This project was made for ESP32-S2
 * with Arduino ESP32 from the 2.0 series
 * Board settings: ESP32S2 Dev Module, UART0, Disabled,
 * Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS), 160MHz (WiFi),
 * QIO, 40MHz, 4MB (32Mb), 921600, None on /dev/ttyUSB0
 */

#include "wifi.h"
#include "config.h"
#include "locoHandling.h"
#include "lowbat.h"
#include "stateMachine.h"
#include "throttleHandling.h"

// #define DEBUG

state wiFredState = STATE_STARTUP;
uint32_t stateTimeout = UINT32_MAX;

void setup() {
// put your setup code here, to run once:

  delay(100);

  Serial.begin(115200);
  Serial.setTimeout(10);
  initConfig();

  initThrottle();
  
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
  
  static uint32_t nextOutput = 0;

  if(nextOutput < millis())
  {
    nextOutput = millis() + 5000;
    log_d("Heap: %d", ESP.getFreeHeap());
  }

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
        switchState(STATE_CONNECTED, TOTAL_NETWORK_TIMEOUT_MS);
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

    case STATE_LOCO_WAITFORTIMEOUT:
      if(WiFi.status() != WL_CONNECTED)
      {
        switchState(STATE_STARTUP);
      }
      if(timeoutReceived() || millis() > stateTimeout)
      {
        switchState(STATE_LOCO_ONLINE);      
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

    case STATE_LOCOS_OFF:
      if(millis() > stateTimeout)
      {
        switchState(STATE_LOWPOWER_WAITING, 100);
      }
      if(!allLocosInactive())
      {
        switchState(STATE_LOCO_ONLINE);
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
  log_d("Old state: %d, New state: %d", wiFredState, newState);
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
