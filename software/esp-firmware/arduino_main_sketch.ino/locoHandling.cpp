#include "locoHandling.h"
#include "config.h"
#include "stateMachine.h"

locoInfo locos[4];
bool locoActive = false;

bool locoRunning[4];

bool inputState[4];
bool inputChanged[4];

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
  static uint32_t debounceCounter;
  static uint8_t switchState[4];
  static uint8_t speedIn = 0, speedOut = 0;
  static bool reverseIn, reverseOut;
  static bool functions[MAX_FUNCTION + 1];

  static WiFiClient client;

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
        }
        else
        {
          switchState[i]++;
        }
      }
      else if (digitalRead(inputPins[i]) == HIGH && inputState[i] == true)
      {
        switchState[i] = 0;
        inputState[i] = false;
        inputChanged[i] = true;
      }
    }
  }

  if (!locoActive)
  {
    return;
  }

  switch (locoState)
  {
    case LOCO_OFFLINE:
      if (wiFredState == STATE_CONNECTED)
      {
        if (client.connect(locoServer.name, locoServer.port))
        {
          client.setNoDelay(true);
          client.setTimeout(10);
          locoState = LOCO_CONNECTED;
        }
      }
      break;

    case LOCO_CONNECTED:
    case LOCO_ONLINE:
      break;

  }
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

