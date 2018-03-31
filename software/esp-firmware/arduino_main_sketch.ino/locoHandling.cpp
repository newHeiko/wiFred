#include "locoHandling.h"
#include "config.h"

locoInfo locos[4];
bool locoActive = false;

bool locoRunning[4];

bool e_allLocosOff;

bool inputState[4];
bool inputChanged[4];

serverInfo locoServer;

void locoInit(void)
{
  pinMode(LOCO1_INPUT, INPUT_PULLUP);
  pinMode(LOCO2_INPUT, INPUT_PULLUP);
  pinMode(LOCO3_INPUT, INPUT_PULLUP);
  pinMode(LOCO4_INPUT, INPUT_PULLUP);

  delay(20);

  if(digitalRead(LOCO1_INPUT) == LOW)
  {
    inputState[0] = true;
  }
  if(digitalRead(LOCO2_INPUT) == LOW)
  {
    inputState[1] = true;
  }
  if(digitalRead(LOCO3_INPUT) == LOW)
  {
    inputState[2] = true;
  }
  if(digitalRead(LOCO4_INPUT) == LOW)
  {
    inputState[3] = true;
  }
}

void locoHandler(void)
{
  static uint32_t debounceCounter;
  static uint8_t switchState[4];

  // debounce inputs every 10ms
  if(millis() > debounceCounter)
  {
    debounceCounter += 10;

    if(digitalRead(LOCO1_INPUT) == LOW && inputState[0] == false)
    {
      if(switchState[0] >= 4)
      {
        inputState[0] = true;
        inputChanged[0] = true;
      }
      else
      {
        switchState[0]++;
      }
    }
    else if(digitalRead(LOCO1_INPUT) == HIGH && inputState[0] == true)
    {
        switchState[0] = 0;
        inputState[0] = false;
        inputChanged[0] = true;      
    }

    if(digitalRead(LOCO2_INPUT) == LOW && inputState[1] == false)
    {
      if(switchState[1] >= 4)
      {
        inputState[1] = true;
        inputChanged[1] = true;
      }
      else
      {
        switchState[1]++;
      }
    }
    else if(digitalRead(LOCO2_INPUT) == HIGH && inputState[1] == true)
    {
        switchState[1] = 0;
        inputState[1] = false;
        inputChanged[1] = true;      
    }

    if(digitalRead(LOCO3_INPUT) == LOW && inputState[2] == false)
    {
      if(switchState[2] >= 4)
      {
        inputState[2] = true;
        inputChanged[2] = true;
      }
      else
      {
        switchState[2]++;
      }
    }
    else if(digitalRead(LOCO3_INPUT) == HIGH && inputState[2] == true)
    {
        switchState[2] = 0;
        inputState[2] = false;
        inputChanged[2] = true;      
    }

    if(digitalRead(LOCO4_INPUT) == LOW && inputState[3] == false)
    {
      if(switchState[3] >= 4)
      {
        inputState[3] = true;
        inputChanged[3] = true;
      }
      else
      {
        switchState[3]++;
      }
    }
    else if(digitalRead(LOCO4_INPUT) == HIGH && inputState[3] == true)
    {
        switchState[3] = 0;
        inputState[3] = false;
        inputChanged[3] = true;      
    }
  }
}

bool getInputState(uint8_t input)
{
  return inputState[input];
}

bool getInputChanged(uint8_t input)
{
  if(inputChanged[input])
  {
    inputChanged[input] = false;
    return true;
  }
  else
  {
    return false;
  }
}

