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
  static uint32_t timeout;
  static uint32_t debounceCounter;
  static uint8_t switchState[4];
  static uint8_t speedIn = 0, speedOut = 0;
  static bool reverseIn, reverseOut;

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
        if (client.connect(locoServer.name, locoServer.port))
        {
          client.setNoDelay(true);
          client.setTimeout(10);
          locoState = LOCO_CONNECTED;
          timeout = millis() + 1000;
          currentLoco = 0;
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
<<<<<<< HEAD
          String id = String(mac[0], 16) + String(mac[1], 16) + String(mac[2], 16) + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);
=======
          String id = String(mac[0], 16) + String(mac[5], 16);
>>>>>>> 49316d23beaadc1c54f8dabc742400930f06bd94
          client.print("HU" + id + "\n");
          client.print(String("N") + throttleName + "\n");
        }
        else if (line.startsWith("*"))
        {
          client.print("*+");
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
      }
      break;
    case LOCO_ONLINE:

      // send any information coming from the keypad/potentiometer
      client.print(handleThrottle());

      if (!client.connected())
      {
        locoState = LOCO_OFFLINE;
        setLEDvalues("0/0", "0/0", "25/50");
      }
      break;
  }
}

/**
   Acquire a new loco for this throttle, including function setting according to function infos

   Will return the same value if needs to be called more than once, loco + 1 if finished
*/
uint8_t requestLoco(uint8_t loco)
{
  static uint8_t step = 0;
  switch (step)
  {
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
      break;
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

