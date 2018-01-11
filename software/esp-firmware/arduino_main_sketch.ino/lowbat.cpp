#include <ESP8266WiFi.h>

#include "lowbat.h"

bool lowBattery;

void lowBatteryInit(void)
{

}

void lowBatteryHandler(void)
{
  #if NUM_AD_SAMPLES > 64
  #warning "Increase data type size of buffer for more than 64 samples"
  #endif
  static uint16_t buffer = 0;
  static uint8_t index = 0;

  buffer += analogRead(A0);

  index++;
  if(index == NUM_AD_SAMPLES)
  {
    // calculate battery voltage in millivolts ( * 2 to account for for hardware voltage divider)
    uint16_t value = buffer / NUM_AD_SAMPLES * 2;
    if(value < LOW_BATTERY_MILLIVOLTS)
    {
      lowBattery = true;
    }
    if(value < EMPTY_BATTERY_MILLIVOLTS)
    {
      // shut down entirely
      #warning "Re-enable Deep Sleep when connected to battery"
      ESP.deepSleep(0);
    }
    buffer = index = 0;
  }
}

