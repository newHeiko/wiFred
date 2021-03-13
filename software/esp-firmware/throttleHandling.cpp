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
 * This file handles reading in keys and pushbuttons as well as setting LEDs
 * and reading in the speed potentiometer
 */

#include <stdbool.h>
#include <Ticker.h>

#include "locoHandling.h"
#include "stateMachine.h"
#include "lowbat.h"
#include "throttleHandling.h"

/**
 * LED handling tickers
 */
Ticker ledStopTickerOn;
Ticker ledStopTickerOff;
Ticker ledFwdTickerOn;
Ticker ledFwdTickerOff;
Ticker ledRevTickerOn;
Ticker ledRevTickerOff;

/**
 * LED cycle times
 */
unsigned int ledStopOnTime;
unsigned int ledFwdOnTime;
unsigned int ledRevOnTime;

/**
 * Input handling ticker
 */
Ticker debounceInput;

/**
 * Current state of inputs
 */
bool inputState[17] = { false };
bool inputPressed[17] = { false };

/**
 * Potentiometer value for zero speed (counterclockwise limit)
 */
unsigned int potiMin;

/**
 * Potentiometer value for max speed (clockwise limit)
 */
unsigned int potiMax;

/**
 * Battery voltage readout factor
 * Multiply readout by this value to correct it
 */
float battFactor = 1.0f;

/**
 * turn LED off
 */
void ledOff(int ledPin)
{
  digitalWrite(ledPin, HIGH);
}

/**
 * turn LED on and schedule turning it off again
 */
void ledOn(int ledPin)
{
  switch(ledPin)
  {
    case LED_STOP:
      if(ledStopOnTime > 0)
        {
          digitalWrite(ledPin, LOW);
          ledStopTickerOff.once_ms(10 * ledStopOnTime, ledOff, LED_STOP);
        }
      break;
    case LED_FWD:
      if(ledFwdOnTime > 0)
      {
        digitalWrite(ledPin, LOW);
        ledFwdTickerOff.once_ms(10 * ledFwdOnTime, ledOff, LED_FWD);
      }
      break;
    case LED_REV:
      if(ledRevOnTime > 0)
      {
        digitalWrite(ledPin, LOW);
        ledRevTickerOff.once_ms(10 * ledRevOnTime, ledOff, LED_REV);
      }
      break;
  }
}

/**
 * Remember current direction setting
 */
bool reverseOut = false;

/**
 * Change LED timers
 */
void setLEDvalues(String ledFwd, String ledRev, String ledStop)
{
  static String oldLedFwd = "";
  static String oldLedRev = "";
  static String oldLedStop = "";

  if(oldLedFwd != ledFwd || oldLedRev != ledRev || oldLedStop != ledStop)
  {
    alignas(4) unsigned int onTime;
    alignas(4) unsigned int cycleTime;

    if(sscanf(ledStop.c_str(), "%u/%u", &onTime, &cycleTime) == 2)
    {
      ledStopTickerOn.attach_ms(10 * cycleTime, ledOn, LED_STOP);
      ledStopOnTime = onTime;
      if(ledStopOnTime > 0)
      {
        ledOn(LED_STOP);
      }
      else
      {
        ledOff(LED_STOP);
      }
    }

    if(sscanf(ledFwd.c_str(), "%u/%u", &onTime, &cycleTime) == 2)
    {
      ledFwdTickerOn.attach_ms(10 * cycleTime, ledOn, LED_FWD);
      ledFwdOnTime = onTime;
      if(ledFwdOnTime > 0)
      {
        ledOn(LED_FWD);
      }
      else
      {
        ledOff(LED_FWD);
      }
    }

    if(sscanf(ledRev.c_str(), "%u/%u", &onTime, &cycleTime) == 2)
    {
      ledRevTickerOn.attach_ms(10 * cycleTime, ledOn, LED_REV);
      ledRevOnTime = onTime;
      if(ledRevOnTime > 0)
      {
        ledOn(LED_REV);
      }
      else
      {
        ledOff(LED_REV);
      }
    }

    oldLedStop = ledStop;
    oldLedFwd = ledFwd;
    oldLedRev = ledRev;
  }
}

/**
 * Get state of input buttons
 * 
 * @param the key to query
 * @return true if input button is pressed (pin value is low)
 */
bool getInputState(keys key)
{
  return inputState[key];
}

/**
 * Get input state changes from input button
 * 
 * @param the key to query
 * @returns true if button has been pressed since last call
 */
bool getInputPressed(keys key)
{
  if(inputPressed[key])
  {
    inputPressed[key] = false;
    return true;
  }
  return false;
}

/**
 * Callback function for debouncing keys
 */
void debounceInputCallback(void)
{
  static uint8_t counter[17] = {};

  for(uint8_t index = KEY_F0; index <= KEY_LOCO4; index++)
  {
    if(digitalRead(KEY_PIN[index]) == LOW && inputState[index] == false)
    {
      if(counter[index] >= 4)
      {
        inputState[index] = true;
        inputPressed[index] = true;
        counter[index] = 0;
      }
      else
      {
        counter[index]++;
      }
    }
    else if(digitalRead(KEY_PIN[index]) == HIGH && inputState[index] == true)
    {
      if (counter[index] >= 4)
      {
        inputState[index] = false;
        counter[index] = 0;
      }
      else
      {
        counter[index]++;
      }
    }
    else
    {
      counter[index] = 0;
    }
  }
}

/**
 * Periodically check keys for new user input and react accordingly
 */
void handleThrottle(void)
{
  for(unsigned int index = KEY_F0; index <= KEY_LOCO4; index++)
  {
    if(getInputPressed((keys) index))
    {
      Serial.println(String("Key ") + index + " has been pressed.");
    }
  }

  return;
  
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
                setReverse(false);
                setSpeed((uint8_t) speedIn);
              }
              else if(direction == 'R')
              {
                setReverse(true);
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
          switchState(STATE_LOWPOWER_WAITING, 1000);
        }
        break;
      }
  }
}

/**
 * Initialize key settings and LED timer settings
 */
void initThrottle(void)
{
  // Set all LED ports as outputs
  pinMode(LED_STOP, OUTPUT);
  pinMode(LED_FWD, OUTPUT);
  pinMode(LED_REV, OUTPUT);
  pinMode(FLASHLIGHT, OUTPUT);
  
  // Set all key inputs to pullup, all loco selection switch inputs to floating
  for(int k = KEY_F0; k <= KEY_REV; k++)
  {
    pinMode(KEY_PIN[k], INPUT_PULLUP); 
  }
  for(int k = KEY_LOCO1; k <= KEY_LOCO4; k++)
  {
    pinMode(KEY_PIN[k], INPUT);
  }
  
  // Run timer to debounce keys
  debounceInput.attach_ms(10, debounceInputCallback);
}
