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
 * Remember ESTOP setting
 */
bool eSTOP = true;

/**
 * Remember current direction setting
 */
bool reverseOut = false;

/**
 * Send LED settings to AVR - Strings are of the shape "20/100" meaning 20*10ms on time and 100*10ms total cycle time
 */
void setLEDvalues(String led1, String led2, String led3)
{
  static uint32_t timeout = 0;
  static String oldLed1 = "";
  static String oldLed2 = "";
  static String oldLed3 = "";
  if(!locoActive)
  {
    return;
  }

  if(oldLed1 != led1)
  {
    Serial.println("L1:" + led1);
    oldLed1 = led1;
  }
  else if(oldLed2 != led2)
  {
    Serial.println("L2:" + led2);
    oldLed2 = led2;
  }
  else if(oldLed3 != led3)
  {
    Serial.println("L3:" + led3);
    oldLed3 = led3;
  }
  else if(millis() > timeout)
  {
    Serial.println("L1:" + led1);
    Serial.println("L2:" + led2);
    Serial.println("L3:" + led3);

    timeout = millis() + 5000;
  }
  Serial.flush();
}

/**
 * Set current throttle status to ESTOP
 */
void setESTOP(void)
{
  eSTOP = true;
}

/**
 * Get current direction - returns true when reverse
 */
bool getReverse(void)
{
  return reverseOut;
}

/**
 * Periodically check serial port for new information from the AVR
 * 
 * Return a string to be sent to wiThrottle server, may include multiple newlines
 */
String handleThrottle(void)
{
  alignas(4) static uint8_t speedIn = 0;
  static uint8_t speedOut = 0;
  static bool reverseIn;

  String ret = "";

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
        if(wiFredState == STATE_CONNECTED)
        {
          // disconnect and start config mode
          ret += String("MTA*<;>X\n");
          setESTOP();
          ret += "Q\n";
          switchState(STATE_CONFIG_STATION_COMING, 100);
        }
        else if(wiFredState == STATE_CONFIG_STATION)
        {
          shutdownWiFiConfigSTA();
          switchState(STATE_CONNECTING);
        }
      }
      break;

    // Speed and direction command received
    case 'S':
      alignas(4) char direction;
      if(sscanf(inputLine.c_str(), "S:%u:%c", &speedIn, &direction) == 2)
      {
        if(speedIn <= 126)
        {
          if(direction == 'F')
          {
            reverseIn = false;
          }
          else if(direction == 'R')
          {
            reverseIn = true;
          }
          else
          {
            setESTOP();
          }
        }
        else
        {
          speedIn = speedOut;
          setESTOP();
        }
      }
      break;

    // Function command received
    case 'F':
      uint8_t f;
      char upDown;
      if(sscanf(inputLine.c_str(), "F%u_%c", (unsigned int *) &f, &upDown) == 2)
      {
        bool firstLoco = true;
        for(uint8_t l = 0; l < 4 && f <= MAX_FUNCTION; l++)
        {
          if(locos[l].functions[f] == THROTTLE)
          {
            if(upDown == 'D')
            {
              ret += String("MTA") + locoThrottleID[l] + "<;>F1" + f + "\n";
              // if this is the first loco which uses this function
              if(firstLoco)
              {
                firstLoco = false;
                // remember function status to match new locos
                if(globalFunctionStatus[f] == ALWAYS_ON)
                {
                  globalFunctionStatus[f] = ALWAYS_OFF;
                }
                else if(globalFunctionStatus[f] == ALWAYS_OFF)
                {
                  globalFunctionStatus[f] = ALWAYS_ON;
                }
              }
            } 
            else if(upDown == 'U')
            {
              ret += String("MTA") + locoThrottleID[l] + "<;>F0" + f + "\n";
            }
          }
        }
      }
      break;
  }
  
  // Set ESTOP on direction change when not stopped, else copy actual direction and set correct directions for all locos
  if(reverseIn != reverseOut)
  {
    if(speedIn != 0)
    {
      setESTOP();
    }
    else
    {
      reverseOut = reverseIn;
      for(uint8_t l = 0; l < 4; l++)
      {
        if(reverseOut ^ locos[l].reverse)
        {
          ret += String("MTA") + locoThrottleID[l] + "<;>R0\n";
        }
        else
        {
          ret += String("MTA") + locoThrottleID[l] + "<;>R1\n";          
        }
      }
    }
  }

  // Send correct LED values
  if(wiFredState == STATE_CONFIG_STATION)
  {
    setLEDvalues("200/200", "200/200", "200/200");
    // make sure no locos move while or after someone configures the throttle on the WiFi system
    setESTOP();
  }
  else
  {
    String ledForward, ledReverse;
    if(lowBattery)
    {
      ledForward = "50/100";
    }
    else
    {
      ledForward = "100/100";
    }
    if(eSTOP)
    {
      ledReverse = "10/100";
    }
    else
    {
      ledReverse = "0/100";
    }
    if(reverseOut)
    {
      setLEDvalues(ledForward, ledReverse, "0/100");
    }
    else
    {
      setLEDvalues(ledReverse, ledForward, "0/100");
    }
  }

  // Remove eSTOP setting on zero speed if set
  if(eSTOP)
  { 
    static uint32_t timeout = 0;
    if(millis() > timeout)
      {
        ret += String("MTA*<;>X\n");
        timeout = millis() + 1000;
      }
    if(speedIn == 0)
    {
      eSTOP = false;
    }
  }
  // If eSTOP not set, send speed command if there is a new speed or if timeout has been reached
  else
  {
    static uint32_t timeout = 0;
    if(millis() > timeout || speedIn != speedOut)
    {
      speedOut = speedIn;
      ret += String("MTA*<;>V") + speedOut + "\n";
      timeout = millis() + 2000;
    }
  }
  return ret;
}

