#include "wifi.h"
#include "config.h"
#include "locoHandling.h"
#include "lowbat.h"

#define DEBUG

void setup() {
// put your setup code here, to run once:

  initConfig();
  locoInit();
  initClock();
  lowBatteryInit();
  
  Serial.begin(115200);
  #ifdef DEBUG
  Serial.setDebugOutput(true);
  #endif
  delay(100);

  initWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  handleWiFi();
  clockHandler();
  lowBatteryHandler();
  locoHandler();
}

