#include "wifi.h"
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

  initConfig();
  locoInit();
  initClock();
  lowBatteryInit();
  
  Serial.begin(115200);
  Serial.setTimeout(10);
  #ifdef DEBUG
  Serial.setDebugOutput(true);
  #endif
  delay(100);

  initWiFi();

  // properly setup LEDs
  setLEDvalues("0/0", "0/0", "100/200");
}

void loop() {
  // put your main code here, to run repeatedly:
  handleWiFi();
  clockHandler();
  lowBatteryHandler();

  static uint32_t test = 0;

#ifdef DEBUG
  if(test < millis())
  {
    test = millis() + 5000;
    Serial.println(ESP.getFreeHeap());
  }
#endif

  switch(wiFredState)
  {
    case STATE_STARTUP:
      if(!clockActive || getInputState(0) == true || getInputState(1) == true || getInputState(2) == true || getInputState(3) == true)
      {
        initWiFiSTA();
        switchState(STATE_CONNECTING, 60 * 1000);
        setLEDvalues("0/0", "0/0", "100/200");
      }
      else
      {
        initWiFiAP();
        switchState(STATE_CONFIG_AP);
        setLEDvalues("0/0", "0/0", "200/200");
      }
      break;
      
    case STATE_CONNECTING:
      if(WiFi.status() == WL_CONNECTED)
      {
        switchState(STATE_CONNECTED);
        setLEDvalues("0/0", "0/0", "25/50");
      }
      else if(millis() > stateTimeout)
      {
        initWiFiAP();
        switchState(STATE_CONFIG_AP);
        setLEDvalues("0/0", "0/0", "200/200");
      }
      break;

    case STATE_CONNECTED:
      if(getInputState(0) == false && getInputState(1) == false && getInputState(2) == false && getInputState(3) == false)
      {
        if(clockActive)
        {
          initWiFiConfigSTA();
          switchState(STATE_CONFIG_STATION_WAITING, 30 * 1000);
          setLEDvalues("200/200", "200/200", "200/200");
        }
        else
        {
          switchState(STATE_LOWPOWER_WAITING, 30 * 1000);
          setLEDvalues("0/0", "0/0", "1/250");
        }
        break;
      }

      if(WiFi.status() != WL_CONNECTED)
      {
        initWiFiSTA();
        switchState(STATE_CONNECTING, 30 * 1000);
        setLEDvalues("0/0", "0/0", "100/200");
      }
      break;

    case STATE_CONFIG_STATION_WAITING:
      if(millis() > stateTimeout)
      {
        shutdownWiFiSTA();
        switchState(STATE_LOWPOWER);
        setLEDvalues("0/0", "0/0", "1/250");
        break;
      }
    // break;
    // intentional fall-through
    case STATE_CONFIG_STATION:
      if(clockActive && (getInputState(0) == true || getInputState(1) == true || getInputState(2) == true || getInputState(3) == true))
      {
        shutdownWiFiConfigSTA();
        switchState(STATE_CONNECTED);
        setLEDvalues("0/0", "0/0", "25/50");
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
      if(getInputState(0) == true || getInputState(1) == true || getInputState(2) == true || getInputState(3) == true)
      {
         switchState(STATE_STARTUP);
         setLEDvalues("0/0", "0/0", "100/200");
      }
      break;
      
    case STATE_CONFIG_AP:
    // no way to get out of here except for restart
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

