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
 * This file handles the connection to the AVR handling keys, direction switch 
 * and speed input and also forms proper wiThrottle commands from them to be
 * sent to the server via the functions in locoHandling.*
 */

#include <stdbool.h>

#include "locoHandling.h"
#include "stateMachine.h"
#include "lowbat.h"

/**
 * Remember current direction setting
 */
bool reverseOut = false;

/**
 * String keeping the AVR firmware revision
 */
char * avrRevision = NULL;

/**
 * Send LED settings to AVR - Strings are of the shape "20/100" meaning 20*10ms on time and 100*10ms total cycle time
 */
void setLEDvalues(String led1, String led2, String led3)
{
  static String oldLed1 = "";
  static String oldLed2 = "";
  static String oldLed3 = "";
  static uint8_t led1countdown = 0;
  static uint8_t led2countdown = 0;
  static uint8_t led3countdown = 0;

  if(oldLed1 != led1)
  {
    led1countdown = 2;
    oldLed1 = led1;
  }
  if(oldLed2 != led2)
  {
    led2countdown = 2;
    oldLed2 = led2;
  }
  if(oldLed3 != led3)
  {
    led3countdown = 2;
    oldLed3 = led3;
  }


  if(led1countdown > 0)
  {
    Serial.println("L1:" + led1);
    led1countdown--;
  }
  if(led2countdown > 0)
  {
    Serial.println("L2:" + led2);
    led2countdown--;
  }
  if(led3countdown > 0)
  {
    Serial.println("L3:" + led3);
    led3countdown--;
  }
  Serial.flush();  
}

/**
 * Periodically check serial port for new information from the AVR and react accordingly
 */
void handleThrottle(void)
{
  // initialization for avr revision
  if(avrRevision == NULL)
  {
    avrRevision = strdup("unknown");
  }

  // try to set direction
  setReverse(reverseOut);
  
  // if there is input on the serial port
  while(Serial.available() > 0)
  {
    // Parse input from AVR and react appropriately
    String inputLine = Serial.readStringUntil('\n');
    switch(inputLine.charAt(0))
    {
      // ESTOP command received
      case 'E':
        if(inputLine.charAt(6) == 'D')
        {
          setESTOP();
        }
        break;
  
      // CONF command received
      case 'C':
        if(inputLine.charAt(5) == 'D')
        {
          if(wiFredState == STATE_LOCO_ONLINE || wiFredState == STATE_CONNECTED || wiFredState == STATE_LOCO_CONNECTING)
          {
            // disconnect and start config mode
            setESTOP();
            locoDisconnect();
            initWiFiConfigSTA();
            switchState(STATE_CONFIG_STATION_WAITING, 120 * 1000);
          }
          else if(wiFredState == STATE_CONFIG_STATION || wiFredState == STATE_CONFIG_STATION_WAITING)
          {
            shutdownWiFiConfigSTA();
            switchState(STATE_STARTUP, 60 * 1000);
          }
        }
        break;
  
      // Speed and direction command received
      case 'S':
        {
          alignas(4) char direction;
          alignas(4) unsigned int speedIn;
          if(sscanf(inputLine.c_str(), "S:%u:%c", &speedIn, &direction) == 2)
          {
            if(speedIn <= 126)
            {
              if(direction == 'F')
              {
                reverseOut = false;
                setSpeed((uint8_t) speedIn);
              }
              else if(direction == 'R')
              {
                reverseOut = true;
                setSpeed((uint8_t) speedIn);
              }
              else
              {
                setESTOP();
              }
            }
            else
            {
              setESTOP();
            }
          }
        }
        break;
  
      // Function command received
      case 'F':
        {
          alignas(4) unsigned int f;
          alignas(4) char upDown;
          if(sscanf(inputLine.c_str(), "F%u_%c", &f, &upDown) == 2)
          {
            if(upDown == 'D')
            {
              setFunction((uint8_t) f);
            }
            else if(upDown == 'U')
            {
              clearFunction((uint8_t) f);
            }
          }
        }
        break;
  
      // Command to add a loco received
      case '+':
        {
          uint8_t l = inputLine.substring(2).toInt();
          if(l >= 1 && l <= 4)
          {
            if(locoState[l-1] != LOCO_ACTIVE)
            {
              locoState[l-1] = LOCO_ACTIVATE;
            }
          }
        }
        break;
    
      // Command to remove a loco received
      case '-':
        {
          uint8_t l = inputLine.substring(2).toInt();
          if(l >= 1 && l <= 4)
          {
            if(locoState[l-1] != LOCO_INACTIVE)
            {
              locoState[l-1] = LOCO_DEACTIVATE;
            }
          }
        }
        break;
    
      // Battery voltage received
      case 'V':
        batteryVoltage = inputLine.substring(2).toInt();
        break;
  
      // Battery information received
        case 'B':
        if(inputLine.charAt(1) == 'L')
        {
          lowBattery = true;
        }
        else if(inputLine.charAt(1) == 'E')
        {
          lowBattery = true;
          emptyBattery = true;
        }
        else
        {
          lowBattery = false;
          emptyBattery = false;
        }
        break;
  
      // Power Down command received
      case 'P':
        if(inputLine.startsWith("PWR_DOWN"))
        {
          setESTOP();
          locoDisconnect();
          switchState(STATE_LOWPOWER_WAITING, 500);
        }
        break;
      
      // Firmware revision received
      case 'R':
        if(avrRevision != NULL)
        {
          free(avrRevision);
        }
        avrRevision = strdup(inputLine.substring(1).c_str());
        break;       
    }
  }
}
