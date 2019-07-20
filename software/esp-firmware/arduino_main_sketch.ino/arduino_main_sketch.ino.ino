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
 * This file ties everything together to initialize the hardware and
 * form the main loop.
 * 
 * This project was made for ESP12E/ESP12F with Arduino ESP version 2.5.0
 * Board settings: Generic ESP8266 Module, 80MHz, Flash, Disabled, ck, 26MHz,
 * 40MHz, DOUT (compatible), 4M (1M SPIFFS), 2, v1.4 Higher Bandwidth, 
 * Disabled, None, Only Sketch, 115200 on /dev/ttyUSB0
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

  initConfig();
  locoInit();
  lowBatteryInit();
  
  Serial.begin(115200);
  Serial.setTimeout(10);
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
  lowBatteryHandler();

#ifdef DEBUG
  static uint32_t test = 0;

  if(test < millis())
  {
    test = millis() + 5000;
    Serial.println(ESP.getFreeHeap());
  }
#endif

  // send keep alive message on serial port
  static uint32_t keepAliveTimeout;
  if(millis() > keepAliveTimeout)
  {
    Serial.println("KeepAlive");
    keepAliveTimeout = millis() + 5000;
  }

  switch(wiFredState)
  {
    case STATE_STARTUP:
      setLEDvalues("0/0", "0/0", "100/200");
      initWiFiSTA();
      switchState(STATE_CONNECTING, 60 * 1000);
      break;
      
    case STATE_CONNECTING:
      setLEDvalues("0/0", "0/0", "100/200");
      if(WiFi.status() == WL_CONNECTED)
      {
        switchState(STATE_CONNECTED);
      }
      else if(millis() > stateTimeout)
      {
        initWiFiAP();
        switchState(STATE_CONFIG_AP);
      }
      break;

    case STATE_CONNECTED:
      if(getInputState(0) == false && getInputState(1) == false && getInputState(2) == false && getInputState(3) == false)
      {
        switchState(STATE_LOWPOWER_WAITING, 30 * 1000);
        break;
      }

      if(WiFi.status() != WL_CONNECTED)
      {
        initWiFiSTA();
        switchState(STATE_CONNECTING, 30 * 1000);
      }
      break;

    case STATE_CONFIG_STATION_WAITING:
      setLEDvalues("200/200", "200/200", "200/200");
      if(millis() > stateTimeout)
      {
        shutdownWiFiSTA();
        switchState(STATE_LOWPOWER);
        break;
      }
    // break;
    // intentional fall-through
    case STATE_CONFIG_STATION:
      setLEDvalues("200/200", "200/200", "200/200");
      break;

    case STATE_CONFIG_STATION_COMING:
      setLEDvalues("200/200", "200/200", "200/200");
      if(millis() > stateTimeout)
      {
         initWiFiConfigSTA();
         switchState(STATE_CONFIG_STATION);
      }
      break;

    case STATE_LOWPOWER_WAITING:
      setLEDvalues("0/0", "0/0", "1/250");
      if(millis() > stateTimeout)
      {
        shutdownWiFiSTA();
        switchState(STATE_LOWPOWER);
      }
    // break;
    // intentional fall-through
    case STATE_LOWPOWER:
      setLEDvalues("0/0", "0/0", "1/250");
      if(getInputState(0) == true || getInputState(1) == true || getInputState(2) == true || getInputState(3) == true)
      {
         switchState(STATE_STARTUP);
      }
      break;
      
    case STATE_CONFIG_AP:
    // no way to get out of here except for restart
      setLEDvalues("0/0", "0/0", "200/200");
      break;
  }

  locoHandler();
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

