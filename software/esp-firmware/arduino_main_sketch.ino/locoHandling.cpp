#include "locoHandling.h"
#include "config.h"

locoInfo locos[4];
bool locoActive = false;

bool locoRunning[4];

bool e_allLocosOff;

serverInfo locoServer;

void locoInit(void)
{
  pinMode(LOCO1_INPUT, INPUT_PULLUP);
  pinMode(LOCO2_INPUT, INPUT_PULLUP);
  pinMode(LOCO3_INPUT, INPUT_PULLUP);
  pinMode(LOCO4_INPUT, INPUT_PULLUP);
}

void locoHandler(void)
{
  static uint32_t debounceCounter;
  static int8_t switchState[4];

  // debounce inputs every 10ms
  if(millis() > debounceCounter)
  {
    debounceCounter += 10;

//    if(
  }
}

