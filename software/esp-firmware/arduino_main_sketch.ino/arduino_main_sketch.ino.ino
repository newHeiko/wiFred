#include <ESP8266WiFi.h>
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
  delay(100);

  initWiFi();  
}

void loop() {
  // put your main code here, to run repeatedly:
  handleWiFi();
  yield();
  clockHandler();
  yield();
  lowBatteryHandler();
  yield();
  locoHandler();
}
