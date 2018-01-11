#include <ESP8266WiFi.h>
#include "wifi.h"
#include "locoHandling.h"
#include "clockHandling.h"
#include "config.h"
#include "Ticker.h"

t_wlan wlan;

WiFiServer server(80);

#define DEBUG

void initWiFi(void)
{
  // check inputs of loco selection switches at startup and 
  // start in normal mode if at least one is selected
  if(digitalRead(LOCO1_INPUT) == LOW || digitalRead(LOCO2_INPUT) == LOW ||
     digitalRead(LOCO3_INPUT) == LOW || digitalRead(LOCO4_INPUT) == LOW || true)
     {
        WiFi.mode(WIFI_STA);

        delay(100);
        
        #ifdef DEBUG
        Serial.print("Attempting connection to ");
        Serial.print(wlan.ssid);
        Serial.print(" with key ");
        Serial.println(wlan.key);
        #endif

        WiFi.begin(wlan.ssid, wlan.key);
        
        for(uint8_t i=0; i<20; i++)
        {
          delay(500);
          if(WiFi.status() == WL_CONNECTED)
          {
            WiFi.mode(WIFI_STA);
            #ifdef DEBUG
            Serial.print("Successfully connected, local IP ");
            Serial.println(WiFi.localIP());
            #endif
            break;
          }
        }
     }

  if(WiFi.status() != WL_CONNECTED)
  {
    // open an AP for configuration if connection failed above
    WiFi.mode(WIFI_AP);
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String ssid = "wfred-config" + String(mac[0], 16) + String(mac[5], 16);
    #ifdef DEBUG
    Serial.print("Not connected, setting up AP at ");
    Serial.println(ssid);
    #endif
    WiFi.softAP(ssid.c_str());
  }
  // start configuration webserver
  server.begin();

}

void writeMainPage(WiFiClient c)
{
  char startupString[9];
  snprintf(startupString, sizeof(startupString)/sizeof(startupString[0]), "%02d:%02d:%02d", startupTime.hours, startupTime.minutes, startupTime.seconds);

  String resp = String("HTTP/1.1 200 OK\r\n")
              + "Content-Type: text/html\r\n"
              + "Connection: close\r\n\r\n"
              + "<!DOCTYPE HTML>\r\n"
              + "<html><head><title>wfred, WILMA and wfred-clock configuration page</title></head>\r\n"
              + "<body><h1>wfred, WILMA and wfred-clock configuration page</h1>\r\n"
              + "<hr>WiFi and throttle configuration<hr>\r\n"
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Throttle name (max " + String(sizeof(throttleName)/sizeof(throttleName[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"throttleName\" value=\"" + throttleName + "\"></td></tr>"
              + "<tr><td>WiFi SSID (max " + String(sizeof(wlan.ssid)/sizeof(wlan.ssid[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"wifi.ssid\" value=\"" + wlan.ssid + "\"></td></tr>"
              + "<tr><td>WiFi PSK (max " + String(sizeof(wlan.key)/sizeof(wlan.key[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"wifi.key\" value=\"" + wlan.key + "\"></td></tr>"
              + "<input type=\"hidden\" name=\"dummy\" value=\"dummy\">"
              + "<tr><td colspan=2><input type=\"submit\"></td></tr></table></form>\r\n"

              + "<hr>Clock configuration<hr>\r\n"
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Enabled?</td><td><input type=\"checkbox\" name=\"clock.enabled\"" + (clockActive ? " checked" : "") + "></td></tr>"
              + "<tr><td>Clock server and port: </td>"
                + "<td>http://<input type=\"text\" name=\"clock.serverName\" value=\"" + clockServer.name + "\">:<input type=\"text\" name=\"clock.serverPort\" value=\"" + clockServer.port + "\">/json/time</td></tr>"
              + "<tr><td>Startup time (format: H:M:S):</td><td><input type=\"text\" name=\"clock.startUp\" value=\"" + startupString + "\"></td></tr>"
              + "<tr><td>Clock offset from UTC (hours):</td><td><input type=\"text\" name=\"clock.offset\" value=\"" + clockOffset + "\"></td></tr>"
              + "<tr><td>Startup clock rate:</td><td><input type=\"text\" name=\"clock.startupRate\" value=\"" + startupTime.rate10 / 10.0 + "\"></td></tr>"
              + "<tr><td>Maximum clock rate:</td><td><input type=\"text\" name=\"clock.maxClockRate\" value=\"" + clockMaxRate + "\"></td></tr>"
              + "<tr><td>Pulse length for clock (milliseconds):</td><td><input type=\"text\" name=\"clock.pulseLength\" value=\"" + clockPulseLength + "\"></td></tr>"
              + "<tr><td colspan=2><input type=\"submit\"></td></tr></table></form>\r\n";
  c.print(resp);

  resp        = String("<hr>Loco configuration<hr>\r\n")
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Enabled?</td><td><input type=\"checkbox\" name=\"loco.enabled\"" + (locoActive ? " checked" : "") + "></td></tr>"
              + "<tr><td>Loco server and port: </td>"
                + "<td><input type=\"text\" name=\"loco.serverName\" value=\"" + locoServer.name + "\">:<input type=\"text\" name=\"loco.serverPort\" value=\"" + locoServer.port + "\"></td></tr>";
                
  for(uint8_t i=0; i<4; i++)
  {
    resp      += String("<tr><td>Loco ") + (i+1) + " address:</td><td><input type=\"text\" name=\"loco.address" + (i+1) + "\" value=\"" + locos[i].address + "\"></td></tr>"
              + "<tr><td>Reverse? <input type=\"checkbox\" name=\"loco.reverse" + (i+1) + "\"" + (locos[i].reverse ? " checked" : "" ) + "></td>"              + "<td><a href=\"funcmap" + (i+1) + ".html\">Function mapping</a></td></tr>";
  }
  resp        += String("<tr><td colspan=2><input type=\"submit\"></td></tr></table></form>\r\n")
              + "<hr>Restart device (to reconnect to WLAN as configured above)<form action=\"restart.html\" method=\"get\"><input type=\"submit\"></form><hr>\r\n"
              + "</body></html>";

  c.print(resp);
}

void readString(char * dest, size_t maxLength, String input, const char * filter, size_t filterLength)
{
  if(input.indexOf(filter) != -1)
  {
    size_t pos = input.indexOf(filter);
    size_t endpos = input.indexOf("&", pos);
    strncpy(dest, input.substring(pos + filterLength, endpos).c_str(), maxLength - 1);
    dest[maxLength - 1] = '\0';
  }
}

int32_t readInteger(String input, const char * filter, size_t filterLength)
{
  if(input.indexOf(filter) != -1)
  {
    size_t pos = input.indexOf(filter);
    return input.substring(pos+filterLength).toInt();
  }
  return -1;
}

void handleWiFi(void)
{
  WiFiClient client = server.available();
  if(client)
  {
    while(!client.available())
    {
      yield();
    }

    String req = client.readStringUntil('\r');

#ifdef DEBUG
    Serial.println(req);
#endif

    // request is function mapping page
    if(req.indexOf("funcmap") != -1)
    {
      size_t pos = req.indexOf("funcmap");
      uint8_t loco = req.substring(pos + sizeof("funcmap") - 1).toInt();

      for(uint8_t i=0; i<10 && i <= MAX_FUNCTION; i++)
      {
        char filter[] = "f0";
        snprintf(filter, sizeof(filter)/sizeof(filter[0]), "f%d", i);
        if(req.indexOf(filter) != -1)
        {
          locos[loco-1].functions[i] = (functionInfo) readInteger(req, filter, sizeof(filter)/sizeof(filter[0]));
        }
      }

      for(uint8_t i=10; i <= MAX_FUNCTION; i++)
      {
        char filter[] = "f10";
        snprintf(filter, sizeof(filter)/sizeof(filter[0]), "f%d", i);
        if(req.indexOf(filter) != -1)
        {
          locos[loco-1].functions[i] = (functionInfo) readInteger(req, filter, sizeof(filter)/sizeof(filter[0]));
        }
      }

      saveLocoConfig(false);

      String resp = String("HTTP/1.1 200 OK\r\n")
                  + "Content-Type: text/html\r\n"
                  + "Connection: close\r\n\r\n"
                  + "<!DOCTYPE HTML>\r\n"
                  + "<html><head><title>wfred and WILMA function configuration page</title></head>\r\n"
                  + "<body><h1>wfred and WILMA function configuration page</h1>\r\n";
      client.print(resp);
      if(loco < 1 || loco > 4)
      {
        resp      = String("Loco ") + loco + " is not valid. Valid locos are in the range [1..4].";
        client.print(resp);
      }
      else
      {
        resp      = String("<hr>Function configuration for loco ") + loco + " (DCC address: " + locos[loco-1].address + ")<hr>"
                  + "<form action=\"funcmap" + loco + ".html\" method=\"get\"><table border=0>";
        client.print(resp);
        for(uint8_t i=0; i<=MAX_FUNCTION; i++)
        {
          resp    = String("<tr><td>Function ") + i + ":</td>"
                  + "<td><input type=\"radio\" name=\"f" + i + "\" value=\"" + ALWAYS_ON + "\"" 
                    + (locos[loco-1].functions[i] == ALWAYS_ON ? " checked" : "" ) + ">Always On</td>"
                  + "<td><input type=\"radio\" name=\"f" + i + "\" value=\"" + THROTTLE + "\"" 
                    + (locos[loco-1].functions[i] == THROTTLE ? " checked" : "" ) + ">Throttle controlled</td>"
                  + "<td><input type=\"radio\" name=\"f" + i + "\" value=\"" + ALWAYS_OFF + "\"" 
                    + (locos[loco-1].functions[i] == ALWAYS_OFF ? " checked" : "" ) + ">Always Off</td><tr>";
          client.print(resp); 
        }
        resp      = String("<tr><td colspan=4><input type=\"submit\"></td></tr></table></form>\r\n");
        client.print(resp);
      }
      resp        = String("<hr><a href=\"index.html\">Back to main configuration page (unsaved data will be lost)</a><hr></body></html>");
      client.print(resp);
    }
    // request is about restarting the ESP
    else if(req.indexOf("restart.html") != -1)
    {
      ESP.restart();
    }
    // everything else will be caught by general configuration
    else
    {
      // request is about general configuration
      if(req.indexOf("throttleName") != -1)
      {
        readString(throttleName, sizeof(throttleName)/sizeof(throttleName[0]), req, "throttleName", sizeof("throttleName"));
        readString(wlan.ssid, sizeof(wlan.ssid)/sizeof(wlan.ssid[0]), req, "wifi.ssid", sizeof("wifi.ssid"));
        readString(wlan.key, sizeof(wlan.key)/sizeof(wlan.key[0]), req, "wifi.key", sizeof("wifi.key"));

        saveGeneralConfig();
      }

      // request is about clock configuration
      if(req.indexOf("clock.") != -1)
      {
        if(req.indexOf("clock.enabled=on") != -1)
        {
          clockActive = true;
        }
        else
        {
          clockActive = false;
        }
        clockMaxRate = readInteger(req, "clock.maxClockRate", sizeof("clock.maxClockRate"));
        clockPulseLength = readInteger(req, "clock.pulseLength", sizeof("clock.pulseLength"));
        clockOffset = readInteger(req, "clock.offset", sizeof("clock.offset"));
        {
          size_t pos = req.indexOf("clock.startUp");
          uint8_t hours, minutes, seconds;
          hours = req.substring(pos+sizeof("clock.startUp")).toInt();
          pos = req.indexOf("%3A", pos);
          minutes = req.substring(pos + sizeof("%3A") - 1).toInt();
          pos = req.indexOf("%3A", pos + 1);
          seconds = req.substring(pos + sizeof("%3A") - 1).toInt();      
          {
            if(hours < 24 && minutes < 60 && seconds < 60)
            {
              startupTime.hours = hours;
              startupTime.minutes = minutes;
              startupTime.seconds = seconds;
            }
          }
        }
        {
          size_t pos = req.indexOf("clock.startupRate");
          startupTime.rate10 = (uint8_t) 10 * req.substring(pos+sizeof("clock.startupRate")).toFloat();
          if(startupTime.rate10 > 10 * clockMaxRate)
          {
            startupTime.rate10 = 10 * clockMaxRate;
          }
        }
        readString(clockServer.name, sizeof(clockServer.name)/sizeof(clockServer.name[0]), req, "clock.serverName", sizeof("clock.serverName"));
        clockServer.port = readInteger(req, "clock.serverPort", sizeof("clock.serverPort"));        

        saveClockConfig();
      }

      // request is about loco configuration
      if(req.indexOf("loco.") != -1)
      {
        if(req.indexOf("loco.enabled=on") != -1)
        {
          locoActive = true;
        }
        else
        {
          locoActive = false;
        }
        readString(locoServer.name, sizeof(locoServer.name)/sizeof(locoServer.name[0]), req, "loco.serverName", sizeof("loco.serverName"));
        locoServer.port = readInteger(req, "loco.serverPort", sizeof("loco.serverPort"));

        for(uint8_t i=0; i<4; i++)
        {
          {
            char filter[] = "loco.address0";
            snprintf(filter, sizeof(filter)/sizeof(filter[0]), "loco.address%d", i+1);
            locos[i].address=readInteger(req, filter, sizeof(filter)/sizeof(filter[0]));
            if(locos[i].address > 9999 || locos[i].address < 0)
            {
              locos[i].address = -1;
            }
          }

          {
            char filter[] = "loco.reverse0=on";
            snprintf(filter, sizeof(filter)/sizeof(filter[0]), "loco.reverse%d=on", i+1);
            if(req.indexOf(filter) != -1)
            { 
              locos[i].reverse = true;
            } 
            else
            {
              locos[i].reverse = false;
            }
          }
        }
        saveLocoConfig();
      }

      // whatever we read above, respond with general page
      writeMainPage(client);
    }
    // request is about function mapping
    delay(1);
  }
}

