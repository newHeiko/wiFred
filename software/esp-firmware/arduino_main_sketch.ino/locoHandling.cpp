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
 * This file provides functions for connecting to a wiThrottle server and
 * communicating with it, including adding and removing locos and controlling
 * them.
 */

#include "locoHandling.h"
#include "lowbat.h"
#include "config.h"
#include "stateMachine.h"
#include "throttleHandling.h"

locoInfo locos[4];
bool locoActive = false;

// String keeping the Loco Address plus its prefix (L or S)
String locoThrottleID[4];

/**
 * Remember ESTOP setting
 */
bool eSTOP = true;

/**
 * Timeout to re-send ESTOP command
 */
uint32_t eStopTimeout = 0;

/**
 * Timeout to re-send SPEED command if unchanged
 */
uint32_t speedTimeout = 0;

/**
 * Client used to connect to wiThrottle server
 */
WiFiClient client;

/**
 * Speed of all currently attached locos
 */
uint8_t speed = 0;

/**
 * Current "reverse setting" sent out to all locos
 */
bool myReverse = false;

/**
 * wiThrottle server and port to connect to
 */
serverInfo locoServer;

/**
 * Events from throttle
 */
eLocoState locoState[4] = { LOCO_INACTIVE, LOCO_INACTIVE, LOCO_INACTIVE, LOCO_INACTIVE };

/**
 * Remember status of functions
 */
functionInfo globalFunctionStatus[MAX_FUNCTION + 1] = { UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN };

void locoInit(void)
{
  pinMode(LOCO1_INPUT, INPUT_PULLUP);
  pinMode(LOCO2_INPUT, INPUT_PULLUP);
  pinMode(LOCO3_INPUT, INPUT_PULLUP);
  pinMode(LOCO4_INPUT, INPUT_PULLUP);

  delay(50);

  for (uint8_t i = 0; i < 4; i++)
  {
    if (digitalRead(inputPins[i]) == LOW)
    {
      inputState[i] = true;
    }
  }
}

void locoHandler(void)
{
  static uint32_t timeout = 0;
  static uint32_t debounceCounter;
  static uint8_t switchState[4];

  // debounce inputs every 10ms
  if (millis() > debounceCounter)
  {
    debounceCounter += 10;

    for (uint8_t i = 0; i < 4; i++)
    {
      if (digitalRead(inputPins[i]) == LOW && inputState[i] == false)
      {
        if (switchState[i] >= 4)
        {
          inputState[i] = true;
          inputChanged[i] = true;
          switchState[i] = 0;
        }
        else
        {
          switchState[i]++;
        }
      }
      else if (digitalRead(inputPins[i]) == HIGH && inputState[i] == true)
      {
        if (switchState[i] >= 4)
        {
          inputState[i] = false;
          inputChanged[i] = true;
          switchState[i] = 0;
        }
        else
        {
          switchState[i]++;
        }
      }
      else
      {
        switchState[i] = 0;
      }
    }
  }

  if(emptyBattery)
  {
    setESTOP();
    bool allInactive = true;
    for(uint8_t loco = 0; loco < 4; loco++)
    {
      if(locoState[loco] != LOCO_INACTIVE)
      {
        locoState[loco] = LOCO_DEACTIVATE;
        allInactive = false;
      }
    }
    if(allInactive)
    {
      locoDisconnect();
      switchState(STATE_LOWPOWER_WAITING, 100);
      return;
    }
  }
  
  if(!client.connected())
  {
    for(uint8_t loco = 0; loco < 4; loco++)
    {
      if(locoState[loco] == LOCO_ACTIVE)
      {
        locoState[loco] = LOCO_ACTIVATE;
      }
    }
    switchState(STATE_CONNECTED, 60 * 1000);
    Serial.println("OF");
    return;
  }

  // send ESTOP command if requested
  if(eSTOP && millis() > eStopTimeout)
  {
    client.print("MTA*<;>X\n");
    eStopTimeout += 5000;
  }
  
  switch(wiFredState)
  {
    case STATE_LOCO_ONLINE:
      // remove ESTOP setting if speed is zero
      if(speed == 0)
      {
        setLEDvalues("0/0", "0/0", "25/50");
        if(millis() > timeout)
        {
          // flush input and output buffers
          client.flush();
          while(client.read() > -1)
            ;
          client.stop();
          client.setTimeout(1000);
          if (client.connect(locoServer.name, locoServer.port))
          {
            client.setNoDelay(true);
            client.setTimeout(10);
            locoState = LOCO_CONNECTED;
            timeout = millis() + 5000;
          }
          else
          {
            timeout = millis() + 2000;
          }
        }
        eSTOP = false;
      }
      
      // if not in emergency stop, send speed value to all locos
      if(!eSTOP && millis() > speedTimeout)
      {
        client.print(String("MTA*<;>V") + speed + "\n");
        speedTimeout += 5000;
      }
      
      // check if any of the loco selectors have been changed
      for(uint8_t currentLoco = 0; currentLoco < 4; currentLoco++)
      {
        if(locoState[currentLoco] == LOCO_FUNCTIONS)
        {
          requestLocoFunctions(currentLoco);
          break;
        }
        if(locoState[currentLoco] == LOCO_ACTIVATE && locos[currentLoco].address != -1)
        {
          // make sure no loco (currently attached) is moving
          setESTOP();
          requestLoco(currentLoco);
          break;
        }
        else if(locoState[currentLoco] == LOCO_DEACTIVATE)
        {
          setESTOP();
          client.print(String("MTA") + locoThrottleID[currentLoco] + "<;>r\n");
          client.print(String("MT-") + locoThrottleID[currentLoco] + "<;>" + locoThrottleID[currentLoco] + "\n");
          locoState[currentLoco] = LOCO_INACTIVE;
        }
      }
      // flush all input data
      client.flush();
      while (client.read() > -1)
        ;
      break;

    default:
      // flush all input data
      client.flush();
      while (client.read() > -1)
        ;
      break;
  }

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
    ledReverse = "30/50";
  }
  else
  {
    ledReverse = "0/100";
  }
  if(myReverse)
  {
    setLEDvalues(ledReverse, ledForward, "0/100");
  }
  else
  {
    setLEDvalues(ledForward, ledReverse, "0/100");
  }
}

/**
 * Connect to wiThrottle server
 */
void locoConnect(void)
{
  if(client.connect(locoServer.name, locoServer.port))
  {
    client.setNoDelay(true);
    client.setTimeout(10);
    switchState(STATE_LOCO_CONNECTING, 10 * 1000);
  }
}

/**
 * Disconnect from wiThrottle server
 */
void locoDisconnect(void)
{
  for(uint8_t loco = 0; loco < 4; loco++)
  {
    if(locoState[loco] == LOCO_ACTIVE)
    {
      locoState[loco] = LOCO_ACTIVATE;
    }
  }
  client.print("Q\n");
  Serial.println("OF");
}

/**
 * Initialize connection to wiThrottle server with client ID etc. after receiving the greeting message
 */
void locoRegister(void)
{
  if(client.available())
  {
    String line = client.readStringUntil('\n');
    if (line.startsWith("VN2.0"))
    {
      uint8_t mac[6];
      WiFi.macAddress(mac);
      String id = String(mac[0], 16) + String(mac[1], 16) + String(mac[2], 16) + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);
      client.print("HU" + id + "\n");
      client.print(String("N") + throttleName + "\n");
      client.print("*+\n");
      switchState(STATE_LOCO_ONLINE);
      Serial.println("ON");
      // flush all input data
      while (client.read() > -1)
        ;
      client.flush();
    }
  }  
  else if(!client.connected())
  {
    switchState(STATE_CONNECTED, 60 * 1000);
    Serial.println("OF");
  }
}

/**
 * Activate function (only of currently connected and function is throttle controlled)
 */
void setFunction(uint8_t f)
{
  if(wiFredState != STATE_LOCO_ONLINE)
  {
    return;
  }
  bool firstLoco = true;
  for(uint8_t l = 0; l < 4 && f <= MAX_FUNCTION; l++)
  {
    if(locos[l].functions[f] == THROTTLE)
    {
      client.print(String("MTA") + locoThrottleID[l] + "<;>F1" + f + "\n");
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
  }
}

/**
 * Deactivate function (only of currently connected and function is throttle controlled)
 */
void clearFunction(uint8_t f)
{
  if(wiFredState != STATE_LOCO_ONLINE)
  {
    return;
  }
  for(uint8_t l = 0; l < 4 && f <= MAX_FUNCTION; l++)
  {
    if(locos[l].functions[f] == THROTTLE)
    {
      client.print(String("MTA") + locoThrottleID[l] + "<;>F0" + f + "\n");
    }
  }
}

/**
 * Set current direction
 */
void setReverse(bool newReverse)
{
  if(newReverse != myReverse)
  {
    if(speed != 0)
    {
      setESTOP();
    }
    myReverse = newReverse;
    if(wiFredState != STATE_LOCO_ONLINE)
    {
      return;
    }
    for(uint8_t l = 0; l < 4; l++)
    {
      if(locoState[l] != LOCO_ACTIVE)
      {
        continue;
      }
      if(myReverse ^ locos[l].reverse)
      {
        client.print(String("MTA") + locoThrottleID[l] + "<;>R0\n");
      }
      else
      {
        client.print(String("MTA") + locoThrottleID[l] + "<;>R1\n");
      }
    }
  }
}

/**
 * Set current speed - if it is new, remove timeout
 */
void setSpeed(uint8_t newSpeed)
{
  if(speed != newSpeed)
  {
    speed = newSpeed;
    speedTimeout = millis();
  }
}

/**
 * Set current throttle status to ESTOP
 */
void setESTOP(void)
{
  eStopTimeout = millis();
  if(wiFredState == STATE_LOCO_ONLINE && !eSTOP)
  {
    client.print("MTA*<;>X\n");
    eStopTimeout += 5000;
  }
  eSTOP = true;
}

/**
 * Acquire a new loco for this throttle, including function setting according to function infos
 */
void requestLoco(uint8_t loco)
{
  // only act if the loco is valid and active
  if(loco >= 4 || locos[loco].address == -1)
  {
    return;
  }
  // first step for new loco: Send "loco acquire" command and send ESTOP command right afterwards to make sure loco is not moving
  if(locos[loco].longAddress)
  {
    locoThrottleID[loco] = String("L") + locos[loco].address;
  }
  else
  {
    locoThrottleID[loco] = String("S") + locos[loco].address;      
  }
  client.print(String("MT+") + locoThrottleID[loco] + "<;>" + locoThrottleID[loco] + "\n");
  client.print(String("MTA") + locoThrottleID[loco] + "<;>X\n");
  setESTOP();
  locoState[loco] = LOCO_FUNCTIONS;
}

/**
 * Correctly set functions on newly acquired loco
 */
void requestLocoFunctions(uint8_t loco)
{
  String line = client.readStringUntil('\n');
  if(line.startsWith(String("MTA") + locoThrottleID[loco]))
  {
    bool set = false;
    uint8_t f = 0;
    
    switch(line.charAt(6 + locoThrottleID[loco].length()))
    {
      // responding with function status
      case 'F':
        f = line.substring(8 + locoThrottleID[loco].length()).toInt();
        // only work on functions up to our maximum
        if(f > MAX_FUNCTION)
        {
          break;
        }
        if(line.charAt(7 + locoThrottleID[loco].length()) == '1')
        {
          set = true;
        }
        if(locos[loco].functions[f] == THROTTLE)
        {
          // if this is the first loco that has this function controlled by our function keys, copy state
          if(globalFunctionStatus[f] == UNKNOWN)
          {
            if(set)
            {
              globalFunctionStatus[f] = ALWAYS_ON;
            }
            else
            {
              globalFunctionStatus[f] = ALWAYS_OFF;
            }
          }
          // if this is not the first loco, match function status to other locos
          // note: This does not work properly with momentary functions
          else if( (set && globalFunctionStatus[f] == ALWAYS_OFF) || (!set && globalFunctionStatus[f] == ALWAYS_ON) )
          {
            client.print(String("MTA") + locoThrottleID[loco] + "<;>F1" + f + "\n");
            client.print(String("MTA") + locoThrottleID[loco] + "<;>F0" + f + "\n");
          }
        }
        // if the function is not throttle controlled, match function status to requested function status
        // note: This does not work properly with momentary functions
        if( (set && locos[loco].functions[f] == ALWAYS_OFF) || (!set && locos[loco].functions[f] == ALWAYS_ON) )
        {
          client.print(String("MTA") + locoThrottleID[loco] + "<;>F1" + f + "\n");
          client.print(String("MTA") + locoThrottleID[loco] + "<;>F0" + f + "\n");            
        }
        break;

      // responding with direction status - take this as our chance to set correct direction (ignoring the one set before)
      case 'R':
        if(myReverse ^ locos[loco].reverse)
        {
          client.print(String("MTA") + locoThrottleID[loco] + "<;>R0\n");
        }
        else
        {
          client.print(String("MTA") + locoThrottleID[loco] + "<;>R1\n");
        }
        break;

      // last line of regular response, everything should be done by now, so switch to online state and flush client buffer
      case 's':
        locoState[loco] = LOCO_ACTIVE;
        // flush all input data
        client.flush();
        while (client.read() > -1)
          ;
        break;
    }
  }
}
