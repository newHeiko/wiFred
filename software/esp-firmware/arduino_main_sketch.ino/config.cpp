#include <stdbool.h>

#include "config.h"
#include "wifi.h"
#include "locoHandling.h"
#include "clockHandling.h"

bool wifiSaved = false;
bool clockSaved = false;
bool locoSaved = false;

char throttleName[NAME_CHARS];

void eepromReadBlock(uint8_t * dest, const uint16_t src, size_t length)
{
  uint16_t local_src;
  for(local_src = src; local_src < src + length; dest++, local_src++)
  {
    *dest = EEPROM.read(local_src);
  }
}

void eepromWriteBlock(const uint16_t dest, uint8_t * src, size_t length)
{
  uint16_t local_dest;
  for(local_dest = dest; local_dest < dest + length; local_dest++, src++)
  {
    EEPROM.write(local_dest, *src);
  }
}

/*
 * Read all configuration from EEPROM
 */
void initConfig(void)
{
  EEPROM.begin(512);

  // check for valid EEPROM data
  uint8_t id = EEPROM.read(ID);
  if(id != EEPROM_VALID) // EEPROM data not valid
  {
    memcpy(wlan.ssid, "undef", sizeof("undef"));
    memcpy(wlan.key, "undef", sizeof("undef"));
    memcpy(throttleName, "undef", sizeof("undef"));
    memcpy(locoServer.name, "undef", sizeof("undef"));
    memcpy(clockServer.name, "undef", sizeof("undef"));
    locoServer.port = 12090;
    clockServer.port = 12080;

    for(int i=0; i<4; i++)
    {
      locos[i].address = -1;
      locos[i].reverse = false;
    }
    startupTime.hours = 5;
    startupTime.minutes = 30;
    startupTime.seconds = 0;
    startupTime.rate10 = 10;
    clockMaxRate = 10;
    clockPulseLength = 40;
    clockOffset = 2;
  }
  else
  {
    eepromReadBlock((uint8_t *) wlan.ssid, WLAN_SSID, sizeof(wlan.ssid)/sizeof(wlan.ssid[0]));
    wlan.ssid[sizeof(wlan.ssid)/sizeof(wlan.ssid[0]) - 1] = '\0';
    eepromReadBlock((uint8_t *) wlan.key, WLAN_KEY, sizeof(wlan.key)/sizeof(wlan.key[0]));
    wlan.key[sizeof(wlan.key)/sizeof(wlan.key[0]) - 1] = '\0';
    eepromReadBlock((uint8_t *) throttleName, NAME, sizeof(throttleName)/sizeof(throttleName[0]));
    throttleName[sizeof(throttleName)/sizeof(throttleName[0]) - 1] = '\0';

    clockActive = (bool) EEPROM.read(CLOCK_ACTIVE);
    clockMaxRate = EEPROM.read(CLOCK_MAX_RATE);
    clockPulseLength = EEPROM.read(CLOCK_PULSE_LENGTH);
    clockOffset = (int8_t) EEPROM.read(CLOCK_OFFSET);
    eepromReadBlock((uint8_t *) &startupTime, CLOCK_STARTUP, sizeof(startupTime));
    eepromReadBlock((uint8_t *) &clockServer, CLOCK_SERVER, sizeof(clockServer)); 

    locoActive = (bool) EEPROM.read(LOCO_ACTIVE);
    eepromReadBlock((uint8_t *) &locoServer, LOCO_SERVER, sizeof(locoServer)); 
    eepromReadBlock((uint8_t *) &(locos[0]), LOCO1, sizeof(locos[0]));
    eepromReadBlock((uint8_t *) &(locos[1]), LOCO2, sizeof(locos[1]));
    eepromReadBlock((uint8_t *) &(locos[2]), LOCO3, sizeof(locos[2]));
    eepromReadBlock((uint8_t *) &(locos[3]), LOCO4, sizeof(locos[3]));    
  }
}

void saveClockConfig()
{
  EEPROM.write(CLOCK_ACTIVE, clockActive);
  EEPROM.write(CLOCK_MAX_RATE, clockMaxRate);
  EEPROM.write(CLOCK_PULSE_LENGTH, clockPulseLength);
  EEPROM.write(CLOCK_OFFSET, clockOffset);

  eepromWriteBlock(CLOCK_STARTUP, (uint8_t *) &startupTime, sizeof(startupTime));
  eepromWriteBlock(CLOCK_SERVER, (uint8_t *) &clockServer, sizeof(clockServer));

  clockSaved = true;
  
  if(wifiSaved)
  {
    EEPROM.write(ID, EEPROM_VALID);
  }
  EEPROM.commit();
}

void saveGeneralConfig()
{
  eepromWriteBlock(WLAN_SSID, (uint8_t *) wlan.ssid, sizeof(wlan.ssid)/sizeof(wlan.ssid[0]));
  eepromWriteBlock(WLAN_KEY, (uint8_t *) wlan.key, sizeof(wlan.key)/sizeof(wlan.key[0]));
  eepromWriteBlock(NAME, (uint8_t *) throttleName, sizeof(throttleName)/sizeof(throttleName[0]));

  wifiSaved = true;
  if(locoSaved || clockSaved)
  {
    EEPROM.write(ID, EEPROM_VALID);
  }
  EEPROM.commit();
}

void saveLocoConfig(bool mainSave)
{
  EEPROM.write(LOCO_ACTIVE, locoActive);
  eepromWriteBlock(LOCO_SERVER, (uint8_t *) &locoServer, sizeof(locoServer)); 
  eepromWriteBlock(LOCO1, (uint8_t *) &(locos[0]), sizeof(locos[0]));
  eepromWriteBlock(LOCO2, (uint8_t *) &(locos[1]), sizeof(locos[1]));
  eepromWriteBlock(LOCO3, (uint8_t *) &(locos[2]), sizeof(locos[2]));
  eepromWriteBlock(LOCO4, (uint8_t *) &(locos[3]), sizeof(locos[3]));

  if(mainSave)
  {
    locoSaved = true;
    if(wifiSaved)
    {
      EEPROM.write(ID, EEPROM_VALID);
    }
  }
  EEPROM.commit();
}
