/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2021 Heiko Rosemann
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

#define DEBUG

#include <WiFi.h>
#include <ESPmDNS.h>

#include "locoHandling.h"
#include "lowbat.h"
#include "config.h"
#include "stateMachine.h"
#include "throttleHandling.h"

locoInfo locos[4];

/** 
 * String keeping the Loco Address plus its prefix (L or S) 
 */
String locoThrottleID[4];

/**
 * Remember ESTOP setting
 */
bool eSTOP = true;

/**
 * Timeout to send heart-beat
 */
uint32_t lastHeartBeat = 0;

/**
 * Timeout for keep alive
 */
uint32_t keepAliveTimeout = 5000;

/**
 * Speed command holdoff-timer
 */
uint32_t lastSpeedUpdate = 0;


/**
 * Client used to connect to wiThrottle server
 */
WiFiClient client;

/**
 * Speed of all currently attached locos
 */
uint8_t speed = 0;

/**
 * New Speed of all currently attached locos
 */
volatile uint8_t newSpeed = 0;

/**
 * Current "reverse setting" sent out to all locos
 */
bool myReverse = false;

/**
 * wiThrottle server and port to connect to
 */
serverInfo locoServer;

/**
 * If server shall be found automatically, this will be the place to save its name
 */
char * automaticServer;

/**
 * If server shall be found automatically, this will be the place to save its IP address
 */
IPAddress automaticServerIP;

/**
 * Events from throttle
 */
eLocoState locoState[4] = { LOCO_INACTIVE, LOCO_INACTIVE, LOCO_INACTIVE, LOCO_INACTIVE };

/**
 * locoState timeouts
 */
uint32_t locoTimeout[4] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };

/**
 * Remember status of functions
 */
functionInfo globalFunctionStatus[MAX_FUNCTION + 1] = { UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN };

void locoHandler(void)
{
  uint32_t now = millis();

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

  // handle lost connection to server
  if(!client.connected() && !emptyBattery)
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

  // remove ESTOP setting if poti turned to zero
  if(eSTOP && newSpeed == 0)
  {
    eSTOP = false;
  }

  // send new speed, if changed, and past holdoff-period
  if(!eSTOP && speed != newSpeed && now - lastSpeedUpdate >= SPEED_HOLDOFF_PERIOD)
  {
    speed = newSpeed;
    client.print(String("MTA*<;>V") + speed + "\n");
    lastSpeedUpdate = lastHeartBeat = now;
  }

  // sending heart-beat regurarly
  if(now - lastHeartBeat >= keepAliveTimeout)
  {
    client.print("*\n");
    lastHeartBeat = now;
  }
      
  // check if any of the loco selectors have been changed
  for(uint8_t currentLoco = 0; currentLoco < 4; currentLoco++)
  {
    if(locoState[currentLoco] == LOCO_LEAVE_FUNCTIONS)
    {
      setLocoFunctions(currentLoco);
      break;
    }
    else if(locoState[currentLoco] == LOCO_FUNCTIONS)
    {
      getLocoFunctions(currentLoco);
      break;
    }
    else if(locoState[currentLoco] == LOCO_ACTIVATE && locos[currentLoco].address != -1)
    {
      // make sure no loco (currently attached) is moving
      setESTOP();
      requestLoco(currentLoco);
      break;
    }
    else if(locoState[currentLoco] == LOCO_DEACTIVATE)
    {
      if(locos[currentLoco].address != -1)
      {
        setESTOP();
        client.print(String("MT-") + locoThrottleID[currentLoco] + "<;>r\n");
      }

      locoState[currentLoco] = LOCO_INACTIVE;
      if(allLocosInactive())
      {
        switchState(STATE_LOCOS_OFF, 5000);
      }
    }
    else if(currentLoco == 3)
    {
      // if none of the locos had any status change,
      // flush all input data
      client.flush();
      while (client.read() > -1)
        ;
    }
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
  if(locoServer.automatic && automaticServer != nullptr)
    {
      log_d("Trying to connect to automatic server %s...", automaticServer);
      if(client.connect(automaticServerIP, locoServer.port))
	    {
        log_d("...succeeded.");
	      client.setNoDelay(true);
	      client.setTimeout(10);
	      switchState(STATE_LOCO_CONNECTING, 10 * 1000);
	    }
      else
      {
        log_d("...failed. Resetting server info.");
        free(automaticServer);
        automaticServer = nullptr;
      }
    }
  else if(!locoServer.automatic)
    {
      log_d("Trying to connect to server %s...", locoServer.name);
      if(client.connect(locoServer.name, locoServer.port))
	    {
        log_d("...succeeded.");
	      client.setNoDelay(true);
	      client.setTimeout(10);
	      switchState(STATE_LOCO_CONNECTING, 10 * 1000);
	    }
    }

  if(locoServer.automatic && automaticServer == nullptr
     && (wiFredState == STATE_CONNECTED || wiFredState == STATE_CONFIG_AP) )
    {
      log_d("Looking for automatic server.");
      uint32_t n = MDNS.queryService("withrottle", "tcp");
      for(uint32_t i = 0; i < n; i++)
      {
        log_d("Hostname: %s IP: %s Port: %u", MDNS.hostname(i).c_str(), MDNS.IP(i).toString().c_str(), MDNS.port(i));
        if(MDNS.port(i) == locoServer.port)
	      {
	        automaticServer = strdup(MDNS.hostname(i).c_str());
	        automaticServerIP = MDNS.IP(i);
//	        MDNS.removeQuery();
	        break;          
	      }
      }
      if(n == 0)
      {
        automaticServerIP = WiFi.localIP();
        automaticServerIP[3] = 1;
        automaticServer = strdup(automaticServerIP.toString().c_str());
        log_d("No MDNS-announced wiThrottle server found. Trying LNWI/DCCEX at %s.", automaticServer);
      }
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
  if(client.connected())
  {
    client.print(String("N") + throttleName + "\n");
  
    if(client.available())
    {
      String line = client.readStringUntil('\n');
      if (line.startsWith("VN2.0"))
      {
        uint8_t mac[6];
        WiFi.macAddress(mac);
        String id = String(mac[0], 16) + String(mac[1], 16) + String(mac[2], 16) + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);
        client.print("HU" + id + "\n");
        switchState(STATE_LOCO_WAITFORTIMEOUT, 1000);
        Serial.println("ON");
      }
    }
  }
  else
  {
    switchState(STATE_CONNECTED, 60 * 1000);
    Serial.println("OF");
  }
}

/**
 * Wait for timeout in greeting message
 * 
 * @returns true if timeout received
 */
bool timeoutReceived(void)
{
  bool success = false;
  while(client.available())
  {
    String line = client.readStringUntil('\n');
    if(line.charAt(0) == '*')
    {
      keepAliveTimeout = 400 * line.substring(1).toInt();
      client.print("*+\n");
      success = true;
    }
  }
  return success;
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
    // skip inactive locos
    if(locoState[l] != LOCO_ACTIVE)
    {
      continue;
    }
    
    switch(locos[l].functions[f])
    {
      case THROTTLE_SINGLE:
        if(!isOnlyLoco(l))
        {
          break;
        }
      
      // intentionally fall through
      
      case THROTTLE:
      case THROTTLE_LOCKING:
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

      // intentionally fall through
      
      case THROTTLE_MOMENTARY:
        client.print(String("MTA") + locoThrottleID[l] + "<;>F1" + f + "\n");
        break;

      case ALWAYS_ON:
      case ALWAYS_OFF:
      case IGNORE:
        break;
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
    // skip inactive locos
    if(locoState[l] != LOCO_ACTIVE)
    {
      continue;
    }
    
    switch(locos[l].functions[f])
    {
      case THROTTLE_SINGLE:
        if(!isOnlyLoco(l))
        {
          break;
        }
      
      // intentionally fall through
      
      case THROTTLE:
      case THROTTLE_LOCKING:
      case THROTTLE_MOMENTARY:
        client.print(String("MTA") + locoThrottleID[l] + "<;>F0" + f + "\n");
        break;

      case ALWAYS_ON:
      case ALWAYS_OFF:
      case IGNORE:
        break;
    }
  }
}

/**
 * Set current direction - only accepts the direction change if speed is zero
 */
void setReverse(bool newReverse)
{
  if(newReverse != myReverse)
  {
    if(speed != 0)
    {
      setESTOP();
      return;
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
void setSpeed(uint8_t _newSpeed)
{
  newSpeed = _newSpeed;
}

/**
 * Set current throttle status to ESTOP
 */
void setESTOP(void)
{
  if(wiFredState == STATE_LOCO_ONLINE && !eSTOP)
  {
    client.print("MTA*<;>X\n");
  }
  eSTOP = true;
}

/**
 * Acquire a new loco for this throttle, including function setting according to function infos
 */
void requestLoco(uint8_t loco)
{
  // only act if the loco is valid...
  if(loco >= 4)
  {
    return;
  }
  // ...and active
  if(locos[loco].address == -1)
  {
    locoState[loco] = LOCO_ACTIVE;
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
  locoTimeout[loco] = millis() + 500;
}

/**
 * Correctly set functions and direction on newly acquired loco
 */
void setLocoFunctions(uint8_t loco)
{
  for(uint8_t f=0; f<=MAX_FUNCTION; f++)
  {
    // Set function momentary setting correctly
    switch(locos[loco].functions[f])
    {
      case THROTTLE_MOMENTARY:
        client.print(String("MTA") + locoThrottleID[loco] + "<;>m1" + f + "\n");
        break;
      
      case THROTTLE_LOCKING:
      case THROTTLE_SINGLE:
        client.print(String("MTA") + locoThrottleID[loco] + "<;>m0" + f + "\n");
        break;

      case THROTTLE:
      case ALWAYS_ON:
      case ALWAYS_OFF:
      case IGNORE:
        break;
    }

    // Set function to requested state
    switch(locos[loco].functions[f])
    {
      case THROTTLE:
      case THROTTLE_LOCKING:
        if(globalFunctionStatus[f] == ALWAYS_ON)
        {
          client.print(String("MTA") + locoThrottleID[loco] + "<;>f1" + f + "\n");
        }
        if(globalFunctionStatus[f] == ALWAYS_OFF)
        {
          client.print(String("MTA") + locoThrottleID[loco] + "<;>f0" + f + "\n");
        }
        break;

      case ALWAYS_ON:
        client.print(String("MTA") + locoThrottleID[loco] + "<;>f1" + f + "\n");
        break;

      case ALWAYS_OFF:
        client.print(String("MTA") + locoThrottleID[loco] + "<;>f0" + f + "\n");
        break;
            
      case THROTTLE_MOMENTARY:
      case THROTTLE_SINGLE:
      case IGNORE:
        break;
    }
  }

  switch(locos[loco].direction)
  {
    case DIR_NORMAL:
      locos[loco].reverse = false;
      break;

    case DIR_REVERSE:
      locos[loco].reverse = true;
      break;

    case DIR_DONTCHANGE:
      // set in readout
      break;
  }

  // Set correct direction
  if(myReverse ^ locos[loco].reverse)
  {
    client.print(String("MTA") + locoThrottleID[loco] + "<;>R0\n");
  }
  else
  {
    client.print(String("MTA") + locoThrottleID[loco] + "<;>R1\n");
  }

  // flush all client data
  client.flush();
  while(client.read() > -1)
    ;
  locoState[loco] = LOCO_ACTIVE;
}

/**
 * Read functions on newly acquired loco
 */
void getLocoFunctions(uint8_t loco)
{
  if(!client.available())
  {
    if(locoTimeout[loco] < millis())
    {
      locoTimeout[loco] = UINT32_MAX;
      locoState[loco] = LOCO_LEAVE_FUNCTIONS;
    }
  }
  else
  {
    String line = client.readStringUntil('\n');
#ifdef DEBUG
    Serial.println();
    Serial.println(line);
#endif
    if(line.startsWith(String("MTA") + locoThrottleID[loco]))
    {
#ifdef DEBUG
      Serial.println(line);
      Serial.println(locoThrottleID[loco]);
      Serial.println(String("") + loco + " " + line.charAt(6 + locoThrottleID[loco].length()));
#endif

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
          if(locos[loco].functions[f] == THROTTLE || locos[loco].functions[f] == THROTTLE_LOCKING)
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
          }
          break;
  
        // responding with direction status - if this loco should keep its direction, set its reverse parameter accordingly
        case 'R':
          if(locos[loco].direction == DIR_DONTCHANGE)
          {
            locos[loco].reverse = (line.charAt(7 + locoThrottleID[loco].length()) == '0') ^ myReverse;
          }
          break;
  
        // last line of regular response, everything should be done by now, so switch to online state and flush client buffer
        case 's':
          locoState[loco] = LOCO_LEAVE_FUNCTIONS;
          locoTimeout[loco] = UINT32_MAX;
          // flush all input data
          while (client.read() > -1)
            ;
          break;
      }
    }
  }
}

/**
 * Are there any active locos left?
 * 
 * @returns true if all locos have been deactivated
 */
bool allLocosInactive(void)
{
  for(uint8_t l = 0; l < 4; l++)
  {
    if(locoState[l] != LOCO_INACTIVE)
    {
      return false;
    }
  }
  return true;
}

/**
 * Is this the only active loco?
 * 
 * @returns true if all other locos are inactive
 */
bool isOnlyLoco(uint8_t loco)
{
  for(uint8_t l = 0; l < 4; l++)
  {
    if(l == loco)
    {
      continue;
    }
    if(locoState[l] != LOCO_INACTIVE)
    {
      return false;
    }
  }
  return true;
}
