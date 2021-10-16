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
 * This file ties everything together to initialize the hardware and
 * form the main loop.
 * 
 * This project was made for ESP12E/ESP12F with Arduino ESP version 3.0.2
 * Board settings: Generic ESP8266 Module, 2, 115200, 80MHz, 26MHz,
 * 4MB (FS:1MB, OTA:~1019kB), DOUT (compatible), 40MHz, no dtr (aka ck), 
 * Disabled, None, v2 Higher Bandwidth, Flash, Legacy, Only Sketch, 
 * nonos-sdk 2.2.1+119 (191122), Basic SSL, /dev/ttyUSB0
 * 
 * Additionally, it requires the ESPAsyncUDP library from 
 * https://github.com/me-no-dev/ESPAsyncUDP to be installed in the Arduino
 * IDE, i.e. clone the github repo above to your sketchbook/libraries folder.
 */

#include "wifi.h"
#include "config.h"
#include "locoHandling.h"
#include "lowbat.h"
#include "stateMachine.h"
#include "throttleHandling.h"

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

  // request status from AVR
  Serial.println("INIT");

  initWiFi();

}

void loop() {
  // put your main code here, to run repeatedly:
  handleWiFi();
  handleThrottle();

  // check for empty battery
  // only if not online and not on the path for wiFred reset
  // if online, check will be done in locoHandler
  // (to disconnect before power down)
  if(emptyBattery &&
      wiFredState != STATE_LOCO_ONLINE &&
      wiFredState != STATE_WAIT_ON_RED_KEY &&
      wiFredState != STATE_STARTUP)
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
      showVoltageIfOff("0/0", "0/0", "100/200");
      if(getInputState(KEY_ESTOP))
      {
        switchState(STATE_WAIT_ON_RED_KEY, WAIT_ON_KEY_TIMEOUT);
      }
      else if(getInputState(KEY_SHIFT))
      {
        switchState(STATE_WAIT_ON_YELLOW_KEY, WAIT_ON_KEY_TIMEOUT);
      }
      else
      {
        initWiFiSTA();
        switchState(STATE_CONNECTING, TOTAL_NETWORK_TIMEOUT_MS);
      }
      break;
      
    case STATE_CONNECTING:
      showVoltageIfOff("0/0", "0/0", "100/200");
      if(WiFi.status() == WL_CONNECTED)
      {
        initMDNS();
        if(getInputState(KEY_F0))
        {
          switchState(STATE_WAIT_ON_F0_KEY, WAIT_ON_KEY_TIMEOUT);
        }
        else
        {
          broadcastUDP();
          switchState(STATE_CONNECTED, TOTAL_NETWORK_TIMEOUT_MS);
        }
      }
      else if(millis() > stateTimeout)
      {
        initWiFiAP();
        switchState(STATE_CONFIG_AP);
      }
      break;

    case STATE_CONNECTED:
      showVoltageIfOff("0/0", "0/0", "25/50");
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
      showVoltageIfOff("0/0", "0/0", "25/50");
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
      showVoltageIfOff("0/0", "0/0", "25/50");
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
      showVoltageIfOff("200/200", "200/200", "200/200");
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
      showVoltageIfOff("200/200", "200/200", "200/200");
      if(WiFi.status() != WL_CONNECTED)
      {
        initWiFiAP();
        switchState(STATE_STARTUP);
      }
      break;

    case STATE_LOCOS_OFF:
      showVoltage();
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
        switchState(STATE_LOWPOWER, 500);
      }
      break;
    
    case STATE_LOWPOWER:
      //no need to set LED state, it's already done
      //setLEDvalues("0/0", "0/0", "1/250");
      // shut down ESP if low on battery
      if(lowBattery || emptyBattery || millis() > stateTimeout)
      {
        delay(1000);
        ESP.deepSleep(0);
      }
      else if(!allLocosInactive())
      {
        ESP.restart();
      }
      break;
      
    case STATE_CONFIG_AP:
    // no way to get out of here except for restart
      showVoltageIfOff("0/0", "0/0", "200/200");
      break;

    case STATE_WAIT_ON_RED_KEY:
      setLEDvalues("0/0", "25/50", "25/50");
      if(!getInputState(KEY_ESTOP))
      {
        initWiFiSTA();
        switchState(STATE_CONNECTING, TOTAL_NETWORK_TIMEOUT_MS);
      }
      else if(millis() > stateTimeout)
      {
        // delete all configuration info
        deleteAllConfig();
        // wait for key release, then restart
        while(getInputState(KEY_ESTOP))
        {
          setLEDvalues("0/0", "0/0", "0/0");
        }
        ESP.restart();
      }
      break;

    case STATE_WAIT_ON_YELLOW_KEY:
      setLEDvalues("25/50", "0/0", "25/50");
      if(!getInputState(KEY_SHIFT))
      {
        initWiFiSTA();
        switchState(STATE_CONNECTING, TOTAL_NETWORK_TIMEOUT_MS);
      }
      else if(millis() > stateTimeout)
      {
        initWiFiAP();
        switchState(STATE_CONFIG_AP);
      }
      break;

    case STATE_WAIT_ON_F0_KEY:
      setLEDvalues("25/50", "25/50", "0/0");
      if(!getInputState(KEY_F0))
      {
        switchState(STATE_CONNECTED, TOTAL_NETWORK_TIMEOUT_MS);
      }
      else if(millis() > stateTimeout)
      {
        initWiFiConfigSTA();
        switchState(STATE_CONFIG_STATION);
      }
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
