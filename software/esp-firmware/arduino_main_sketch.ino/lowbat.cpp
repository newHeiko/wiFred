#include <ESP8266WiFi.h>

#include "lowbat.h"

bool lowBattery;
uint16_t batteryVoltage;

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

  static uint32_t lastRead = 0;
  static uint32_t lastSent = 0;

  if(millis() > lastSent && lowBattery)
  {
    lastSent = millis() + 5000;
    Serial.println("LowBattery");
  }

  if(millis() > lastRead)
  {
    lastRead += 8000 / NUM_AD_SAMPLES;

    buffer += analogRead(A0);
    index++;
    if(index == NUM_AD_SAMPLES)
    {
      // calculate battery voltage in millivolts ( / 47 * 147 to account for for hardware voltage divider)
      uint16_t value = buffer / NUM_AD_SAMPLES / 47 * 147;
      if(value < LOW_BATTERY_MILLIVOLTS)
      {
        lowBattery = true;
      }
      if(value < EMPTY_BATTERY_MILLIVOLTS)
      {
        // shut down entirely
        ESP.deepSleep(0);
      }
      batteryVoltage = value;
      buffer = index = 0;
    }
  }
}


