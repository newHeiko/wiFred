#include "locoHandling.h"
#include "config.h"
#include "stateMachine.h"
#include "throttleHandling.h"

locoInfo locos[4];
bool locoActive = false;

bool locoRunning[4];

bool inputState[4];
bool inputChanged[4];

WiFiClient client;

const uint8_t inputPins[] = {LOCO1_INPUT, LOCO2_INPUT, LOCO3_INPUT, LOCO4_INPUT};

eLocoState locoState = LOCO_OFFLINE;

serverInfo locoServer;

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

  if (!locoActive)
  {
    return;
  }

  switch (locoState)
  {
      static uint8_t currentLoco;

    case LOCO_OFFLINE:
      if (wiFredState == STATE_CONNECTED)
      {
        if(millis() > timeout)
        {
          // the following line is a workaround for a memory leak bug in arduino 2.4.0/2.4.1: https://github.com/esp8266/Arduino/issues/4497
          client = WiFiClient();
          client.setTimeout(1000);
          if (client.connect(locoServer.name, locoServer.port))
          {
            client.setNoDelay(true);
            client.setTimeout(10);
            locoState = LOCO_CONNECTED;
            currentLoco = 0;
          }
          timeout = millis() + 1000;
        }
      }
      break;

    case LOCO_CONNECTED:
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        if (line.startsWith("VN2.0"))
        {
          // flush all input data
          while (client.read() > -1)
            ;
          uint8_t mac[6];
          WiFi.macAddress(mac);
          String id = String(mac[0], 16) + String(mac[1], 16) + String(mac[2], 16) + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);
          client.print("HU" + id + "\n");
          client.print(String("N") + throttleName + "\n");
          timeout = millis() + 1000;
          locoState = LOCO_ACQUIRING;
        }
      }
      else if (millis() > timeout)
      {
        locoState = LOCO_OFFLINE;
      }
      break;

    case LOCO_ACQUIRING:
      if (getInputState(currentLoco) == false || locos[currentLoco].address == -1)
      {
        currentLoco++;
      }
      else
      {
        currentLoco = requestLoco(currentLoco);
      }
      if (currentLoco >= 4)
      {
        locoState = LOCO_ONLINE;
      }
      else if (millis() > timeout)
      {
        locoState = LOCO_OFFLINE;
        setLEDvalues("0/0", "0/0", "25/50");
      }
      // if the connection is broken, return to connect state
      if (!client.connected())
      {
        locoState = LOCO_OFFLINE;
        setLEDvalues("0/0", "0/0", "25/50");
      }
      break;

    case LOCO_ACQUIRE_SINGLE:
      // acquire loco - if done, switch back to normal state
      if(requestLoco(currentLoco) > currentLoco)
      {
        locoState = LOCO_ONLINE;
      }
      else if(millis() > timeout)
      {
        locoState = LOCO_OFFLINE;        
        setLEDvalues("0/0", "0/0", "25/50");
      }
      // if the connection is broken, return to connect state
      if (!client.connected())
      {
        locoState = LOCO_OFFLINE;
        setLEDvalues("0/0", "0/0", "25/50");
      }
      break;

    case LOCO_ONLINE:
      // send any information coming from the keypad/potentiometer
      client.print(handleThrottle());

      // flush all input data
      while (client.read() > -1)
        ;

      // check if any of the loco selectors have been changed
      for(currentLoco = 0; currentLoco < 4; currentLoco++)
      {
        if(getInputChanged(currentLoco))
        {
          // make sure no loco (currently attached) is moving
          setESTOP();
          // send ESTOP command
          client.print(handleThrottle());
          // if the new state is selected, acquire the new loco and skip out of loop
          if(getInputState(currentLoco))
          {
            locoState = LOCO_ACQUIRE_SINGLE;
            timeout = millis() + 1000;
            break;
          }
          // if not, release loco and remove the loco from the throttle
          else
          {
            client.print(String("MTA") + currentLoco + "<;>r\n");
            if (locos[currentLoco].longAddress)
            {
              client.print(String("MT-") + currentLoco + "<;>L" + locos[currentLoco].address + "\n");
            }
            else
            {
              client.print(String("MT-") + currentLoco + "<;>S" + locos[currentLoco].address + "\n");
            }
          }
        }
      }

      // flush all input data
      while (client.read() > -1)
        ;

      // if the connection is broken, return to connect state
      if (!client.connected())
      {
        locoState = LOCO_OFFLINE;
        setLEDvalues("0/0", "0/0", "25/50");
      }
      break;
  }
}

/**
 * Acquire a new loco for this throttle, including function setting according to function infos
 *
 * Will return the same value if needs to be called more than once, loco + 1 if finished
 */
uint8_t requestLoco(uint8_t loco)
{
  static functionInfo globalFunctionStatus[MAX_FUNCTION + 1] = { UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
                                             UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN };
  static uint8_t step = 0;
  switch (step)
  {
    // first step: Send "loco acquire" command and send ESTOP command right afterwards to make sure loco is not moving
    case 0:
      if (locos[loco].longAddress)
      {
        client.print(String("MT+") + loco + "<;>L" + locos[loco].address + "\n");
        step = 1;
      }
      else
      {
        client.print(String("MT+") + loco + "<;>S" + locos[loco].address + "\n");
        step = 1;
      }
      setESTOP();
      client.print(String("MTA") + loco + "<;>X\n");
      break;

    // second step: Evaluate response
    case 1:
      String line = client.readStringUntil('\n');
      if(line.startsWith(String("MTA") + loco))
      {
        bool set = false;
        uint8_t f = 0;
        
        switch(line.charAt(7))
        {
          // responding with function status
          case 'F':
            f = line.substring(9).toInt();
            // only work on functions up to our maximum
            if(f > MAX_FUNCTION)
            {
              break;
            }
            if(line.charAt(8) == '1')
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
                client.print(String("MTA") + loco + "<;>F1" + f + "\n");
                client.print(String("MTA") + loco + "<;>F0" + f + "\n");
              }
            }
            // if the function is not throttle controlled, match function status to requested function status
            // note: This does not work properly with momentary functions
            if( (set && locos[loco].functions[f] == ALWAYS_OFF) || (!set && locos[loco].functions[f] == ALWAYS_ON) )
            {
                client.print(String("MTA") + loco + "<;>F1" + f + "\n");
                client.print(String("MTA") + loco + "<;>F0" + f + "\n");            
            }
            break;

          // responding with direction status - take this as our chance to set correct direction (ignoring the one set before)
          case 'R':
            if(getReverse() ^ locos[loco].reverse)
            {
              client.print(String("MTA") + loco + "<;>R0\n");
            }
            else
            {
              client.print(String("MTA") + loco + "<;>R1\n");
            }
            break;

          // last line of regular response, everything should be done by now, so switch to next loco and flush client buffer
          case 's':
            loco++;
            step = 0;
            // flush all input data
            while (client.read() > -1)
              ;
            break;
        }
      }
  }
  return loco;
}

bool getInputState(uint8_t input)
{
  return inputState[input];
}

bool getInputChanged(uint8_t input)
{
  if (inputChanged[input])
  {
    inputChanged[input] = false;
    return true;
  }
  else
  {
    return false;
  }
}

