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
      // calculate network-local time difference and re-adjust local clock rate if required
    }
  }
}

