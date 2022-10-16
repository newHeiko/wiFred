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
 * This file handles the connection to the AVR handling keys, direction switch 
 * and speed input and also forms proper wiThrottle commands from them to be
 * sent to the server via the functions in locoHandling.*
 */

#include <Ticker.h>

#include <stdbool.h>

#include "locoHandling.h"
#include "stateMachine.h"
#include "lowbat.h"
#include "throttleHandling.h"

/**
 * Array containing all valid key masks
 */
const uint32_t keys[] = { KEY_F0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_SHIFT, KEY_ESTOP, KEY_REVERSE, KEY_FORWARD, KEY_LOCO1, KEY_LOCO2, KEY_LOCO3, KEY_LOCO4 };

#define FIRST_LOCO_KEY_MASK 13

/**
 * Current state of inputs
 */
uint32_t inputState = 0;
uint32_t inputPressed = 0;
uint32_t inputToggled = 0;

/**
 * Define behavior of center-off-switch
 * 
 * 0 or higher: Function to set when switch at center position
 * -1: Set speed to zero
 * -2: Ignore
 */
int centerFunction;

/**
 * Status of direction switch
 * 
 * true if in center position
 */
bool centerPosition;

/**
 * Ticker for red LED blink function
 */
Ticker blinkTicker;

/**
 * Unlock direction change when in center position
 */
bool directionChangeUnlock = false;

/**
 * Block direction change even at zero speed
 */
bool directionChangeBlocked = false;

/**
 * Timestamp when direction switch was moved into center position
 */
uint32_t enterCenterPositionTime;

/**
 * Blink red LED this many times before resuming normal LED patterns
 * (down counter)
 */
unsigned int blinkLED;

/**
 * Count down blink times
 */
void ledBlinkHandler(void)
{
  if(blinkLED > 0)
  {
    blinkLED--;
  }
  else
  {
    blinkTicker.detach();
    // turn off LED - will hopefully be turned on again soon
    setLEDvalues("", "", "0/10");
  }
}

/**
 * Set red LED blinking numbers
 * 
 * @param number Blink red LED this many times before resuming normal LED patterns
 */
void setLEDblink(unsigned int number)
{
  if(number > 0)
  {
    blinkLED = number - 1;
    Serial.println("L3:10/30");
    Serial.flush();
    blinkTicker.attach_ms(300, ledBlinkHandler);
  }
  else
  {
    blinkTicker.detach();
    // turn off LED - will hopefully be turned o again soon
    setLEDvalues("", "", "0/10");
  }
}

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

  if(oldLed1 != led1 && led1 != "")
  {
    led1countdown = 2;
    oldLed1 = led1;
  }
  if(oldLed2 != led2 && led2 != "")
  {
    led2countdown = 2;
    oldLed2 = led2;
  }
  if(oldLed3 != led3 && led3 != "" && blinkLED == 0)
  {
    led3countdown = 2;
    oldLed3 = led3;
  }

  if(led1countdown > 0)
  {
    Serial.println("L1: " + led1);
    led1countdown--;
  }
  if(led2countdown > 0)
  {
    Serial.println("L2: " + led2);
    led2countdown--;
  }
  if(led3countdown > 0)
  {
    Serial.println("L3: " + led3);
    led3countdown--;
  }
  Serial.flush();
}

/**
 * Allow direction change even if speed > 0
 */
bool allowDirectionChange()
{
  if (directionChangeUnlock && ( (millis() - enterCenterPositionTime) > CENTER_FUNCTION_ESTOP_TIMEOUT) && centerFunction != CENTER_FUNCTION_IGNORE)
  {
    directionChangeUnlock = false;
    return true;
  }
  else
  {
    directionChangeUnlock = false;
    return false;
  }
}

/**
 * Block direction change even if speed == 0
 */
bool blockDirectionChange()
{
  return directionChangeBlocked;
}

/**
 * Get state of input buttons
 * 
 * @param the keymask to query
 * @return true if input button is pressed (pin value is low)
 */
bool getInputState(uint32_t key)
{
  if(inputState & key)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Get input state changes from input button
 * 
 * @param the keymask to query
 * @returns true if button has been pressed since last call
 */
bool getInputPressed(uint32_t key)
{
  if(inputPressed & key)
  {
    inputPressed &= ~key;
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Get input state changes from input button
 *
 * @param the keymask to query
 * @returns true if button has been toggled since last call
*/
bool getInputToggled(uint32_t key)
{
  if(inputToggled & key)
  {
    inputToggled &= ~key;
    return true;
  }
  else
  {
    return false;
  }
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

  // handle direction switch
  if(getInputPressed(KEY_REVERSE))
  {
    reverseOut = true;

    if (0 <= centerFunction && centerFunction <= MAX_FUNCTION)
    {
      clearFunction(centerFunction);
    }

    centerPosition = false;
  }
  if(getInputPressed(KEY_FORWARD))
  {
    reverseOut = false;

    if (0 <= centerFunction && centerFunction <= MAX_FUNCTION)
    {
      clearFunction(centerFunction);
    }

    centerPosition = false;
  }
  if(!getInputState(KEY_REVERSE) && !getInputState(KEY_FORWARD))
  {
    if(!centerPosition)
    {
      enterCenterPositionTime = millis();

      switch(centerFunction)
      {
        case CENTER_FUNCTION_ZEROSPEED:
          if (getSpeed() != 0)
          {
            setSpeed(0);
            directionChangeBlocked = true;
          }
          break;

        case CENTER_FUNCTION_IGNORE:
          break;

        default:
          if(0 <= centerFunction && centerFunction <= MAX_FUNCTION)
          {
            setFunction(centerFunction);
          }
          break;
      }
      directionChangeUnlock = true;
      centerPosition = true;
    }
    if((millis() - enterCenterPositionTime) > CENTER_FUNCTION_ESTOP_TIMEOUT)
    {
      directionChangeBlocked = false;
    }
  }

  // handle f0 key
  if(getInputToggled(KEY_F0))
  {
    if(getInputState(KEY_F0))
    {
      setFunction(0);
    }
    else
    {
      clearFunction(0);
    }
  }
  // handle keys f1 to f8
  {
    // check for shift on press, remember shift for release
    static boolean shift[8] = { false };
    for(int f = 1; f <= 8; f++)
    {
      if(getInputToggled(keys[f]))
      {
        if(getInputState(keys[f]))
        {
          if(getInputState(KEY_SHIFT))
          {
            shift[f - 1] = true;
            setFunction(f + 8);
          }
          else
          {
            shift[f - 1] = false;
            setFunction(f);
          }
        }
        else
        {
          if(shift[f - 1])
          {
            clearFunction(f + 8);
          }
          else
          {
            clearFunction(f);
          }
        }
      }
    }
  }

  // check for new loco switch activity
  for(int l = 0; l < 4; l++)
  {
    if(getInputToggled(keys[FIRST_LOCO_KEY_MASK + l]))
    {
      if(!getInputState(keys[FIRST_LOCO_KEY_MASK + l]) && locoState[l] != LOCO_INACTIVE)
      {
        locoState[l] = LOCO_DEACTIVATE;
      }
      if(getInputState(keys[FIRST_LOCO_KEY_MASK + l]) && locoState[l] != LOCO_ACTIVE)
      {
        locoState[l] = LOCO_ACTIVATE;
      }
    }
  }

  // handle emergency stop key
  if(getInputPressed(KEY_ESTOP))
  {
    setESTOP();
    if(getInputState(KEY_SHIFT))
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
        switchState(STATE_STARTUP, TOTAL_NETWORK_TIMEOUT_MS);
      }
    }
  }

  // if there is input on the serial port
  while(Serial.available() > 0)
  {
    // Parse input from AVR and react appropriately
    String inputLine = Serial.readStringUntil('\n');
    switch(inputLine.charAt(0))
    {
      // Speed and direction command received
      case 'S':
        {
          alignas(4) unsigned int speedIn;
          if(sscanf(inputLine.c_str(), "S:%u", &speedIn) == 1)
          {
            if(speedIn <= 126)
            {
              if(centerFunction == CENTER_FUNCTION_ZEROSPEED && centerPosition)
              {
                speedIn = 0;
              }
              else if(speedIn < 1)
              {
                directionChangeBlocked = false;
              }
              setSpeed(speedIn);
            }
          }
        }
        break;

      // Key state received
      case 'K':
        {
          if(inputLine.length() < 34)
          {
            break;
          }
          uint32_t newKeys = 0;
          for(int bit = 0; bit < 32; bit++)
          {
            if(inputLine.charAt(2 + bit) == '1')
            {
              bitWrite(newKeys, 31 - bit, 1);
            }
            else
            {
              bitWrite(newKeys, 31 - bit, 0);
            }
          }
          inputToggled |= newKeys ^ inputState;
          inputPressed |= newKeys & inputToggled;
          inputState = newKeys;
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
        avrRevision = strdup(inputLine.substring(2).c_str());
        break;
    }
  }
}

/**
 * Show battery voltage through LEDs
 */
void showVoltage(void)
{
  switch((batteryVoltage - 3500) / 100)
  {
    case -2:
    case -1:
    case 0:
    case 1:
      setLEDvalues("0/0", "0/0", "30/50");
      break;
    case 2:
      setLEDvalues("0/0", "0/0", "50/50");
      break;
    case 3:
      setLEDvalues("0/0", "30/50", "50/50");
      break;
    case 4:
      setLEDvalues("0/0", "50/50", "50/50");
      break;
    case 5:
    case 6:
    case 7:
      setLEDvalues("30/50", "50/50", "50/50");
      break;
    default:
      setLEDvalues("50/50", "50/50", "30/50");
      break;
  }
}

/**
 * Show battery voltage through LEDs only if no loco switch is active
 * Show LED values otherwise
 */
void showVoltageIfOff(String ledFwd, String ledRev, String ledStop)
{
  if(getInputState(KEY_LOCO1 | KEY_LOCO2 | KEY_LOCO3 | KEY_LOCO4))
  {
    setLEDvalues(ledFwd, ledRev, ledStop);
    return;
  }
  else
  {
    showVoltage();
  }
}
