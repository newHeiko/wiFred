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
bool inputToggled[17] = { false };

/**
 * A/D conversion ticker
 */
Ticker analogInput;

/**
 * A/D calibration reset ticker
 */
Ticker reduceCalibValues;

/**
 * Flag to show we want to save new config data
 */
volatile boolean saveAnalog = false;

/**
 * Potentiometer value for zero speed (counterclockwise limit)
 */
unsigned int potiMin;

/**
 * Potentiometer value for max speed (clockwise limit)
 */
unsigned int potiMax;

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
    log_d("Changing Led settings: %s, %s, %s", ledFwd, ledRev, ledStop);
    alignas(4) unsigned int onTime;
    alignas(4) unsigned int cycleTime;

    if(sscanf(ledStop.c_str(), "%u/%u", &onTime, &cycleTime) == 2)
    {
      log_d("Change stop: %u out of %u", onTime, cycleTime);
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
      log_d("Change fwd: %u out of %u", onTime, cycleTime);
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
      log_d("Change rev: %u out of %u", onTime, cycleTime);
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
 * Get input state changes from input button
 * 
 * @param the key to query
 * @returns true if button has been toggled since last call
 */
bool getInputToggled(keys key)
{
  if(inputToggled[key])
  {
    inputToggled[key] = false;
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
        inputToggled[index] = true;
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
        inputToggled[index] = true;
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

  // abuse this to set flashlight correctly
  if(inputState[KEY_SHIFT])
  {
    digitalWrite(FLASHLIGHT, HIGH);
  }
  else
  {
    digitalWrite(FLASHLIGHT, LOW);
  }
}

/**
 * Allow direction change even if speed > 0
 */
bool allowDirectionChange()
{
  if(directionChangeUnlock && ( (millis() - enterCenterPositionTime) > CENTER_FUNCTION_ESTOP_TIMEOUT) && centerFunction != CENTER_FUNCTION_IGNORE)
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
 * Periodically check keys for new user input and react accordingly
 */
void handleThrottle(void)
{
  setReverse(reverseOut);
  
  // handle direction switch
  if(getInputPressed(KEY_REV))
  {
    reverseOut = true;
    log_v("Setting direction to reverse");

    if(0 <= centerFunction && centerFunction <= MAX_FUNCTION)
    {
      clearFunction(centerFunction);
    }

    centerPosition = false;
  }
  if(getInputPressed(KEY_FWD))
  {
    reverseOut = false;
    log_v("Setting direction to forward");

    if(0 <= centerFunction && centerFunction <= MAX_FUNCTION)
    {
      clearFunction(centerFunction);
    }

    centerPosition = false;
  }
  if(!getInputState(KEY_REV) && !getInputState(KEY_FWD))
  {
    if(!centerPosition)
    {
      enterCenterPositionTime = millis();

      switch(centerFunction)
      {
        case CENTER_FUNCTION_ZEROSPEED:
          setSpeed(0);
          directionChangeBlocked = true;
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
      log_v("Setting function 0");
      setFunction(0);
    }
    else
    {
      log_v("Releasing function 0");
      clearFunction(0);
    }
  }
  // handle keys f1 to f8
  {
    // check for shift on press, remember shift for release
    static boolean shift[8] = { false };
    for(int f=1; f<=8; f++)
    {
      if(getInputToggled((keys) ((int) KEY_F0 + f)))
      {
        if(getInputState((keys) ((int) KEY_F0 + f)))
        {
          if(getInputState(KEY_SHIFT))
          {
            shift[f-1] = true;
            log_v("Setting function %u", f+8);
            setFunction(f + 8);
          }
          else
          {
            shift[f-1] = false;
            log_v("Setting function %u", f);
            setFunction(f);
          }
        }
        else
        {
          if(shift[f-1])
          {
            log_v("Releasing function %u", f+8);
            clearFunction(f+8);
          }
          else
          {
            log_v("Releasing function %u", f);
            clearFunction(f);
          }
        }
      }
    }
  }

  // check for new loco switch activity
  // inverse logic: getInputState == true means loco switch off
  for(int l=0; l<4; l++)
  {
    if(getInputToggled((keys) ((int) KEY_LOCO1 + l)))
    {
      log_v("Loco selection switch %u changed to %u", l+1, getInputState((keys) ((int) KEY_LOCO1 + l)));
      if(getInputState((keys) ((int) KEY_LOCO1 + l)) && locoState[l] != LOCO_INACTIVE)
      {
        log_v("Deactivating loco %u", l+1);
        locoState[l] = LOCO_DEACTIVATE;
      }
      if(!getInputState((keys) ((int) KEY_LOCO1 + l)) && locoState[l] != LOCO_ACTIVE)
      {
        log_v("Activating loco %u", l+1);
        locoState[l] = LOCO_ACTIVATE;
      }
    }
  }
  
  // handle emergency stop key
  if(getInputPressed(KEY_ESTOP))
  {
    setESTOP();
    log_v("Setting ESTOP");
    if(getInputState(KEY_SHIFT))
    {
      log_v("Shift key active as well - set config mode\n");
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

  static uint32_t nextOutput;
  if(nextOutput < millis())
  {
    log_d("Current battery voltage: %u", batteryVoltage);
    nextOutput = millis() + 5000;
  }

  if(saveAnalog)
  {
    saveAnalogConfig();
    saveAnalog = false;
  }
}

/**
 * Analog to digital converter callback
 */
void adcCallback(void)
{
  static unsigned int counter = 0;
  static uint32_t speedBuffer = 0;
  static uint32_t batteryBuffer = 0;
  static uint8_t oldSpeed = 0;

  if(counter % 2)
  {
    speedBuffer += analogRead(ANALOG_PIN_POTI);
  }
  else
  {
    batteryBuffer += analogReadMilliVolts(ANALOG_PIN_VBATT);
  }
  counter++;

  if(counter >= NUM_SAMPLES * 2)
  {
    static uint32_t newMin = 0;
    static uint32_t newMax = 0;
    static uint8_t newMinCounter = 0;
    static uint8_t newMaxCounter = 0;
    
    speedBuffer /= NUM_SAMPLES;
    
    if(potiMin >= potiMin / 50)
    {
      if(speedBuffer < potiMin - potiMin / 50)
      {
        newMin += speedBuffer;
        newMinCounter++;
        if(newMinCounter >= NUM_OVERSHOOT)
        {
          potiMin = newMin / NUM_OVERSHOOT;
          saveAnalog = true;
          newMin = newMinCounter = 0;
        }
      }
      else
      {
        newMin = newMinCounter = 0;
      }
    }
    if(speedBuffer > potiMax + potiMax / 50)
    {
      newMax += speedBuffer;
      newMaxCounter++;
      if(newMaxCounter >= NUM_OVERSHOOT)
      {
        potiMax = newMax / NUM_OVERSHOOT;
        saveAnalog = true;
        newMax = newMaxCounter = 0;
      }
    }
    else
    {
      newMax = newMaxCounter = 0;
    }
    uint8_t tempSpeed;
    if(speedBuffer > potiMax)
    {
      tempSpeed = 0;
    }
    else if(speedBuffer < potiMin)
    {
      tempSpeed = 253;
    }
    else
    {
      tempSpeed = map(speedBuffer, potiMin, potiMax, 253, 0);
    }
    if(centerFunction == CENTER_FUNCTION_ZEROSPEED && centerPosition)
    {
      tempSpeed = 0;
    }
    else if(tempSpeed < 2)
    {
      directionChangeBlocked = false;
    }
    int8_t delta = tempSpeed - oldSpeed;
    if(delta < -1 || delta > 1)
    {
      log_d("Old speed: %u, new speed: %u", oldSpeed, tempSpeed);
      setSpeed(tempSpeed / 2);
      oldSpeed = tempSpeed;
    }

    batteryBuffer /= NUM_SAMPLES;
    batteryBuffer *= battFactor * 2.0;
    if(batteryBuffer > 4200)
    {
      battFactor = battFactor * 4200.0f / batteryBuffer;
      batteryBuffer = 4200;
      saveAnalog = true;
    }
    if(batteryBuffer < EMPTY_BATTERY_THRESHOLD)
    {
      lowBattery = emptyBattery = true;
    }
    else if(batteryBuffer < LOW_BATTERY_THRESHOLD)
    {
      emptyBattery = true;
    }
    else
    {
      lowBattery = emptyBattery = false;
    }

    batteryVoltage = batteryBuffer;
    batteryBuffer = speedBuffer = counter = 0;
  }
}

/**
 * Periodically reduce potiMax to make sure wiFred always goes to 0
 */
void adcReduce(void)
{
  if(potiMax > 0)
  {
    potiMax--;
  }
  saveAnalog = true;
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
// (inverse logic on getInputState)
  if( getInputState(KEY_LOCO1) && getInputState(KEY_LOCO2) && getInputState(KEY_LOCO3) && getInputState(KEY_LOCO4) )
  {
    showVoltage();
  }
  else
  {
    setLEDvalues(ledFwd, ledRev, ledStop);
  }
}

/**
 * Initialize key settings, analog inputs and LED timer settings
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
    inputState[k] = true;
  }
  
  // Run timer to debounce keys
  debounceInput.attach_ms(10, debounceInputCallback);

  // Run timer to read analog inputs
  analogInput.attach_ms(2, adcCallback);

  // Run timer to recalibrate zero speed
  reduceCalibValues.attach(10, adcReduce);

  // Set ADC attenuation for the two pins used
  analogSetPinAttenuation(ANALOG_PIN_POTI, ADC_11db);
  analogSetPinAttenuation(ANALOG_PIN_VBATT, ADC_11db);
}
