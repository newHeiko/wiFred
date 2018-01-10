#include <string.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

#include "clockHandling.h"
#include "config.h"

uint8_t clockPulseLength = 40;
uint8_t clockMaxRate = 10;
bool clockActive = false;
int8_t clockOffset;

clockInfo ourTime;
clockInfo networkTime;
clockInfo startupTime;
serverInfo clockServer;

volatile bool flagGetTime = true;
volatile bool flagNewTime = false;

Ticker networkSecond;
Ticker ourSecond;
Ticker realSecond;
Ticker oneShot;

void plusOneSecond(clockInfo * input)
{
  input->seconds++;
  if(input->seconds >= 60)
  {
    input->seconds = 0;
    input->minutes++;
  }
  if(input->minutes >= 60)
  {
    input->minutes = 0;
    input->hours++;
  }
  if(input->hours >= 12)
  {
    input->hours = 0;
  }
}

void resetClockOutputs(void)
{
  digitalWrite(CLOCK1_PIN, HIGH);
  digitalWrite(CLOCK2_PIN, HIGH);
}

void secondTickHandler(void)
{
  flagGetTime = true;
}

void setClockOutputs(void)
{
  if(!clockActive)
  {
    return;
  }
  static bool positiveEdge;
  if(positiveEdge)
  {
    digitalWrite(CLOCK1_PIN, LOW);
    positiveEdge = false;
  }
  else
  {
    digitalWrite(CLOCK2_PIN, LOW);
    positiveEdge = true;
  }
  oneShot.once_ms(clockPulseLength, resetClockOutputs);
  plusOneSecond(&ourTime);
  flagNewTime = true;
}

void networkSecondHandler(void)
{
  if(!clockActive)
  {
    return;
  }
  plusOneSecond(&networkTime);
  flagNewTime = true;
}


void initClock(void)
{
  memcpy(&ourTime, &startupTime, sizeof(clockInfo));
  memcpy(&networkTime, &startupTime, sizeof(clockInfo));

  ourTime.hours %= 12;
  networkTime.hours %= 12;

  pinMode(CLOCK1_PIN, OUTPUT);
  pinMode(CLOCK2_PIN, OUTPUT);
  digitalWrite(CLOCK1_PIN, HIGH);
  digitalWrite(CLOCK2_PIN, HIGH);
  
  realSecond.attach(1.0, secondTickHandler);

  if(ourTime.rate10 > 10 * clockMaxRate)
  {
    ourTime.rate10 = 10 * clockMaxRate;
  }

  if(ourTime.rate10 != 0)
  {
    ourSecond.attach(10.0 / ourTime.rate10, setClockOutputs);
  }
  else
  {
    ourSecond.detach();
  }

  if(networkTime.rate10 != 0)
  {
    networkSecond.attach(10.0 / networkTime.rate10, networkSecondHandler);
  }
  else
  {
    networkSecond.detach();
  }
}

void clockHandler(void)
{
  if(!clockActive)
  {
    digitalWrite(CLOCK1_PIN, HIGH);
    digitalWrite(CLOCK2_PIN, HIGH);
  }
  else
  {
    // try to get time from network if we are connected to WLAN
    if(flagGetTime && WiFi.status() == WL_CONNECTED)
    {
      static WiFiClient client;

      if(client.connected())
      {
          if(client.available())
          {
            String line = client.readStringUntil('\n');

            // valid data received
            if(line.indexOf("{\"type\":\"time\"") != -1)
            {
              flagGetTime = false;

              clockInfo temp;
              size_t pos;
              pos = line.indexOf("T");
              temp.hours = line.substring(pos + 1).toInt();
              temp.minutes = line.substring(pos + 4).toInt();
              temp.seconds = line.substring(pos + 7).toInt();

              pos = line.indexOf("rate");
              temp.rate10 = (uint8_t) (line.substring(pos + sizeof("rate") + 1).toFloat() * 10);

              pos = line.indexOf("state");
              uint8_t state = line.substring(pos + sizeof("state") + 1).toInt();
              if(state == 4)
              {
                temp.rate10 = 0;
              }

              if(temp.hours < 24 && temp.minutes < 60 && temp.seconds < 60)
              {
                temp.hours += clockOffset;
                temp.hours %= 12;
                // only change networkSecond timer if rate has changed to avoid glitches
                if(temp.rate10 != networkTime.rate10)
                {
                  if(temp.rate10 != 0)
                  {
                    networkSecond.attach(10.0 / temp.rate10, networkSecondHandler);
                  }
                  else
                  {
                    networkSecond.detach();
                  }
                }
                memcpy(&networkTime, &temp, sizeof(networkTime));
                flagNewTime = true;
              }
            }
          }
      }
      else if(client.connect(clockServer.name, clockServer.port))
      {
        client.setNoDelay(true);
        client.setTimeout(10);
        client.print(String("GET /json/time") + " HTTP/1.1\r\n" +
             "Host: " + clockServer.name + "\r\n" +
             "Connection: close\r\n" +
             "\r\n"
            );
      }      
    }
    if(flagNewTime)
    {
      // reset flag
      flagNewTime = false;

      // keep old rate in case it is not changed
      uint8_t oldRate = ourTime.rate10;
      // calculate network-local time difference and re-adjust local clock rate if required
      int32_t delta = networkTime.hours * 3600 + networkTime.minutes * 60 + networkTime.seconds 
                    - (ourTime.hours * 3600 + ourTime.minutes * 60 + ourTime.seconds);
      // roll over after twelve hours and make sure it's inside [0...12 hours]
      delta %= (12 * 3600);
      if(delta < 0)
      {
        delta += 12 * 3600;
      }

      // simple cases first
      // if we are already at same speed and close enough, keep doing it
      if( (delta < CLOCK_DELTA || delta > 12 * 3600 - CLOCK_DELTA) && networkTime.rate10 == ourTime.rate10)
      {
        // do nothing
      }
      // if we are into a smaller window, set same rate (if we can run as fast)
      else if( (delta < CLOCK_DELTA / 2 || delta > 12 * 3600 - CLOCK_DELTA / 2) && networkTime.rate10 <= clockMaxRate * 10)
      {
        ourTime.rate10 = networkTime.rate10;
      }
      // otherwise check if it will be faster to wait for network time or to play catch-up
      
      // network clock not moving, so we need to catch up
      else if(networkTime.rate10 == 0)
      {
        ourTime.rate10 = clockMaxRate * 10;
      }
      // network clock faster or same than we can run at all, so stop clock
      // (if same, will start running again once network clock has caught up)
      else if(networkTime.rate10 >= clockMaxRate * 10)
      {
        ourTime.rate10 = 0;
      }
      else
      {
        uint8_t maxRateDelta = clockMaxRate * 10 - networkTime.rate10;
        uint16_t catchupTime = delta / maxRateDelta;
        uint16_t waitTime = (12 * 3600 - delta) / networkTime.rate10;
        if(waitTime <= catchupTime)
        {
          ourTime.rate10 = 0;
        }
        else
        {
          ourTime.rate10 = clockMaxRate * 10;
        }
      }

      if(ourTime.rate10 != oldRate)
      {
        if(ourTime.rate10 != 0)
        {
          ourSecond.attach(10.0 / ourTime.rate10, setClockOutputs);
        }
        else
        {
          ourSecond.detach();
        }
      }
    }
  }
}

