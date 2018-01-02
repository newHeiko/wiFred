#include "locoHandling.h"
#include "config.h"

locoInfo locos[4];
bool locoActive = false;

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
  
}

