/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2024 Heiko Rosemann
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
 * with Arduino ESP32 from the 3.0 series
 * 
 * Versions:
 *    Arduino IDE 1.8.19
 *    esp32 3.0.5
 * 
 * Libraries:
 *    ArduinoJson 7.1.0
 * 
 * Board settings:
 *    Board: "ESP32S2 Dev Module"
 *    Upload Speed: "921600"
 *    USB CDC On Boot: "Disabled"
 *    USB Firmware MSC On Boot; "Disabled"
 *    USB DFU On Boot: "Disabled"
 *    Upload Mode: "UART0"
 *    CPU Frequency: "160MHz (WiFI/BT)
 *    Flash Frequency: "40 MHz"
 *    Flash Mode "QIO"
 *    Flash Size: "4MB (32Mb)"
 *    Partion Scheme: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
 *    Core Debug Level: "None"
 *    PSRAM: "Disabled"
 *    Erase All Flash Before Sketch Upload: "Disabled"
 *    Port: "COMxx"
 */
/* Boards:
 * - ESP32S2 Dev Module * esp32:esp32:esp32s2:UploadSpeed=921600,CDCOnBoot=default,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,CPUFreq=160,FlashFreq=40,FlashMode=qio,FlashSize=4M,PartitionScheme=default,DebugLevel=none,PSRAM=disabled,EraseFlash=none,JTAGAdapter=default,CONSOLEBAUD=115200
 */

#include "wifiHandling.h"
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
  
  static uint32_t nextOutput = 0;

  if(nextOutput < millis())
  {
    nextOutput = millis() + 5000;
    log_d("Heap: %d", ESP.getFreeHeap());
  }

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
        switchState(STATE_CONNECTING, TOTAL_NETWORK_TIMEOUT_MS);
        initWiFiSTA();
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
          switchState(STATE_CONNECTED, TOTAL_NETWORK_TIMEOUT_MS);
          broadcastUDP();
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
        locoDisconnect();
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
        shutdownWiFiSTA();
        switchState(STATE_LOWPOWER, 60000);
      }
      if(!allLocosInactive() && !lowBattery && !emptyBattery)
      {
        switchState(STATE_LOCO_ONLINE);
      }
      break;
    
    case STATE_LOWPOWER:
      setLEDvalues("0/0", "0/0", "1/250");
      // shut down ESP if low on battery or inactivity timeout plus delay reached
      if(lowBattery || emptyBattery || millis() > stateTimeout)
      {
        delay(1000);
        ESP.deepSleep(0);
      }
      // restart when user reenables a loco switch
      else if(!allLocosInactive())
      {
        ESP.restart();
      }
      // else wait for RC delay to switch off power
      break;
      
    case STATE_CONFIG_AP:
    // no way to get out of here except for restart
      showVoltageIfOff("0/0", "0/0", "200/200");
      break;

    case STATE_WAIT_ON_RED_KEY:
      setLEDvalues("0/0", "25/50", "25/50");
      if(!getInputState(KEY_ESTOP))
      {
        showVoltageIfOff("0/0", "0/0", "100/200");
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
        showVoltageIfOff("0/0", "0/0", "100/200");
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
