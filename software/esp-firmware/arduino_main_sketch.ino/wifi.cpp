#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "wifi.h"
#include "locoHandling.h"
#include "clockHandling.h"
#include "config.h"
#include "lowbat.h"
#include "Ticker.h"
#include "stateMachine.h"
#include "throttleHandling.h"

#define DEBUG

t_wlan wlan;

ESP8266WebServer server(80);

void readString(char * dest, size_t maxLength, String input)
{
  strncpy(dest, input.c_str(), maxLength - 1);
  dest[maxLength - 1] = '\0';
}

void handleWiFi(void)
{
  server.handleClient();
}

void initWiFiConfigSTA(void)
{
  // change IP address to config page
  // replace last byte in IP address with 253 (configuration IP address)
  IPAddress configIP = WiFi.localIP();;
  configIP[3] = 253;
  WiFi.config(configIP, WiFi.gatewayIP(), WiFi.subnetMask());
}

void shutdownWiFiConfigSTA(void)
{
  // re-enable dhcp
  WiFi.config(0u, 0u, 0u);
}

void initWiFiSTA(void)
{
  WiFi.disconnect();
  WiFi.config(0u, 0u, 0u);

  WiFi.mode(WIFI_STA);
       
  #ifdef DEBUG
  Serial.print("Attempting connection to ");
  Serial.print(wlan.ssid);
  Serial.print(" with key ");
  Serial.println(wlan.key);
  #endif

  WiFi.begin(wlan.ssid, wlan.key);
}

void shutdownWiFiSTA(void)
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void initWiFiAP(void)
{
  // open an AP for configuration if connection failed
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String ssid = "wiFred-config" + String(mac[0], 16) + String(mac[5], 16);
  #ifdef DEBUG
  Serial.println();
  Serial.print("Not connected, setting up AP at ");
  Serial.println(ssid);
  #endif
  WiFi.softAP(ssid.c_str());
}

void writeMainPage()
{
  if(wiFredState == STATE_CONFIG_STATION_WAITING)
  {
    switchState(STATE_CONFIG_STATION);
    setLEDvalues("100/100", "100/100", "100/100");
  }

  // check if this is a "set configuration" request
  if(server.hasArg("throttleName") && server.hasArg("wifi.ssid") && server.hasArg("wifi.key"))
  {
    readString(throttleName, sizeof(throttleName)/sizeof(throttleName[0]), server.arg("throttleName"));
    readString(wlan.ssid, sizeof(wlan.ssid)/sizeof(wlan.ssid[0]), server.arg("wifi.ssid"));
    readString(wlan.key, sizeof(wlan.key)/sizeof(wlan.key[0]), server.arg("wifi.key"));

    saveGeneralConfig();
  }

  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred configuration page</title></head>\r\n"
              + "<body><h1>General configuration</h1>\r\n"
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Throttle name (max " + String(sizeof(throttleName)/sizeof(throttleName[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"throttleName\" value=\"" + throttleName + "\"></td></tr>"
              + "<tr><td>WiFi SSID (max " + String(sizeof(wlan.ssid)/sizeof(wlan.ssid[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"wifi.ssid\" value=\"" + wlan.ssid + "\"></td></tr>"
              + "<tr><td>WiFi PSK (max " + String(sizeof(wlan.key)/sizeof(wlan.key[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"wifi.key\" value=\"" + wlan.key + "\"></td></tr>"
              + "<tr><td colspan=2><input type=\"submit\"></td></tr></table></form>\r\n"

              + "<hr>Clock configuration<hr>\r\n"
              + "<a href=clock.html>Clock configuration subpage</a>\r\n"
              + "<hr>Loco configuration<hr>\r\n"
              + "<a href=loco.html>Loco configuration subpage</a>\r\n"
              + "<hr>Restart system to enable new WiFi settings<hr>\r\n"
              + "<a href=restart.html>Restart system to enable new WiFi settings</a>\r\n"
              + "<hr><hr>Status page<hr>\r\n"
              + "<a href=status.html>wiFred status subpage</a>\r\n"
              + "</body></html>";
  server.send(200, "text/html", resp);
}

void writeClockPage()
{
  #ifdef DEBUG
  Serial.println("Clock page");
  #endif
  // check if this is a "set configuration" request
  if(server.hasArg("clock.serverName") && server.hasArg("clock.serverPort") && server.hasArg("clock.startUp"))
  {
    clockActive = server.hasArg("clock.enabled");
    readString(clockServer.name, sizeof(clockServer.name)/sizeof(clockServer.name[0]), server.arg("clock.serverName"));
    clockServer.port = server.arg("clock.serverPort").toInt();
    String startupString = server.arg("clock.startUp");
    alignas(4) uint8_t hours, minutes, seconds;
    
    if(sscanf(startupString.c_str(), "%u:%u:%u", (unsigned int *) &hours, (unsigned int *) &minutes, (unsigned int *) &seconds) == 3)
    {
      if(hours < 24 && minutes < 60 && seconds < 60)
      {
        startupTime.hours = hours % 12;
        startupTime.minutes = minutes;
        startupTime.seconds = seconds;
      }
    }
    clockOffset = server.arg("clock.offset").toInt();
    clockMaxRate = server.arg("clock.maxClockRate").toInt();
    startupTime.rate10 = (uint8_t) 10 * server.arg("clock.startupRate").toFloat();
    if(startupTime.rate10 > 10 * clockMaxRate)
    {
      startupTime.rate10 = 10 * clockMaxRate;
    }
    clockPulseLength = server.arg("clock.pulseLength").toInt();

    saveClockConfig();
  }
  
  
  char startupString[9];
  snprintf(startupString, sizeof(startupString)/sizeof(startupString[0]), "%02d:%02d:%02d", startupTime.hours, startupTime.minutes, startupTime.seconds);

  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred configuration page</title></head>\r\n"
              + "<body><h1>Clock configuration</h1>\r\n"
              + "<form action=\"clock.html\" method=\"get\"><table border=0>"
              + "<tr><td>Enabled?</td><td><input type=\"checkbox\" name=\"clock.enabled\"" + (clockActive ? " checked" : "") + "></td></tr>"
              + "<tr><td>Clock server and port: </td>"
                + "<td>http://<input type=\"text\" name=\"clock.serverName\" value=\"" + clockServer.name + "\">:<input type=\"text\" name=\"clock.serverPort\" value=\"" + clockServer.port + "\">/json/time</td></tr>"
              + "<tr><td>Startup time (format: H:M:S):</td><td><input type=\"text\" name=\"clock.startUp\" value=\"" + startupString + "\"></td></tr>"
              + "<tr><td>Clock offset from UTC (hours):</td><td><input type=\"text\" name=\"clock.offset\" value=\"" + clockOffset + "\"></td></tr>"
              + "<tr><td>Startup clock rate:</td><td><input type=\"text\" name=\"clock.startupRate\" value=\"" + startupTime.rate10 / 10.0 + "\"></td></tr>"
              + "<tr><td>Maximum clock rate:</td><td><input type=\"text\" name=\"clock.maxClockRate\" value=\"" + clockMaxRate + "\"></td></tr>"
              + "<tr><td>Pulse length for clock (milliseconds):</td><td><input type=\"text\" name=\"clock.pulseLength\" value=\"" + clockPulseLength + "\"></td></tr>"
              + "<tr><td><input type=\"submit\"></td><td><a href=\"/\">Back to main configuration page (unsaved data will be lost)</a></td></tr></table></form>\r\n"
              + "</body></html>";
  server.send(200, "text/html", resp);
}

void writeLocoPage()
{
  // check if this is a "set configuration" request
  if(server.hasArg("loco.serverName") && server.hasArg("loco.serverPort"))
  {
    locoActive = server.hasArg("loco.enabled");
    readString(locoServer.name, sizeof(locoServer.name)/sizeof(locoServer.name[0]), server.arg("loco.serverName"));
    locoServer.port = server.arg("loco.serverPort").toInt();

    locos[0].address = server.arg("loco.address1").toInt();
    locos[1].address = server.arg("loco.address2").toInt();
    locos[2].address = server.arg("loco.address3").toInt();
    locos[3].address = server.arg("loco.address4").toInt();

    locos[0].longAddress = server.hasArg("loco.longAddress1");
    locos[1].longAddress = server.hasArg("loco.longAddress2");
    locos[2].longAddress = server.hasArg("loco.longAddress3");
    locos[3].longAddress = server.hasArg("loco.longAddress4");
    
    for(uint8_t i=0; i<4; i++)
    {
      if(locos[i].address > 10239 || locos[i].address < 0)
      {
        locos[i].address = -1;
      }
      if(!locos[i].longAddress && (locos[i].address > 127 || locos[i].address < 1))
      {
        locos[i].address = -1;
      }
    }

    locos[0].reverse = server.hasArg("loco.reverse1");
    locos[1].reverse = server.hasArg("loco.reverse2");
    locos[2].reverse = server.hasArg("loco.reverse3");
    locos[3].reverse = server.hasArg("loco.reverse4");

    saveLocoConfig();
  }

  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred configuration page</title></head>\r\n"
              + "<body><h1>Loco configuration</h1>\r\n"
              + "<form action=\"loco.html\" method=\"get\"><table border=0>"
              + "<tr><td>Enabled?</td><td><input type=\"checkbox\" name=\"loco.enabled\"" + (locoActive ? " checked" : "") + "></td></tr>"
              + "<tr><td>Loco server and port: </td>"
              + "<td><input type=\"text\" name=\"loco.serverName\" value=\"" + locoServer.name + "\">:<input type=\"text\" name=\"loco.serverPort\" value=\"" + locoServer.port + "\"></td></tr>";
  for(uint8_t i=0; i<4; i++)
  {
    resp      += String("<tr><td>Loco ") + (i+1) + " DCC address: ([1..9999] is valid, -1 to disable)</td><td><input type=\"text\" name=\"loco.address" + (i+1) + "\" value=\"" + locos[i].address + "\"></td></tr>"
              + "<tr><td>Reverse? <input type=\"checkbox\" name=\"loco.reverse" + (i+1) + "\"" + (locos[i].reverse ? " checked" : "" ) + "></td>"               + "<td></td></tr>"
              + "<tr><td>Long Address? <input type=\"checkbox\" name=\"loco.longAddress" + (i+1) + "\"" + (locos[i].longAddress ? " checked" : "" ) + "></td>"  + "<td><a href=\"funcmap.html?loco=" + (i+1) + "\">Function mapping</a></td></tr>";
  }
  resp        += String("<tr><td><input type=\"submit\"></td><td><a href=\"/\">Back to main configuration page (unsaved data will be lost)</a></td></tr></table></form>\r\n")
              + "</body></html>";
  
  server.send(200, "text/html", resp);
}

void writeFuncMapPage()
{
  uint8_t loco = server.arg("loco").toInt();

  // check if this is a "set configuration" request
  if(loco >= 1 && loco <= 4 && server.hasArg("f0") && server.hasArg("f1"))
  {
    for(uint8_t i=0; i<= MAX_FUNCTION; i++)
    {
      locos[loco-1].functions[i] = (functionInfo) server.arg(String("f") + i).toInt();
    }

    saveLocoConfig(false);
  }

  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred configuration page</title></head>\r\n"
              + "<body><h1>Function mapping for Loco: " + loco + "</h1>\r\n";
  if(loco < 1 || loco > 4)
  {
    resp      += String("Loco ") + loco + " is not valid. Valid locos are in the range [1..4].";
  }
  else
  {
    resp      += String("<hr>Function configuration for loco ") + loco + " (DCC address: " + locos[loco-1].address + ")<hr>"
              + "<form action=\"funcmap.html\" method=\"get\"><table border=0>";
    for(uint8_t i=0; i<=MAX_FUNCTION; i++)
    {
      resp    += String("<tr><td>Function ") + i + ":</td>"
              + "<td><input type=\"radio\" name=\"f" + i + "\" value=\"" + ALWAYS_ON + "\"" 
                + (locos[loco-1].functions[i] == ALWAYS_ON ? " checked" : "" ) + ">Always On</td>"
              + "<td><input type=\"radio\" name=\"f" + i + "\" value=\"" + THROTTLE + "\"" 
                + (locos[loco-1].functions[i] == THROTTLE ? " checked" : "" ) + ">Throttle controlled</td>"
              + "<td><input type=\"radio\" name=\"f" + i + "\" value=\"" + ALWAYS_OFF + "\"" 
                + (locos[loco-1].functions[i] == ALWAYS_OFF ? " checked" : "" ) + ">Always Off</td><tr>";
    }
    resp      += String("<tr><td colspan=4><input type=\"hidden\" name=\"loco\" value=\"") + loco + "\"><input type=\"submit\"></td></tr></table></form>\r\n";
  }
  resp        += String("<hr><a href=\"loco.html\">Back to loco configuration page (unsaved data will be lost)</a>")
              + "<hr><a href=\"/\">Back to main configuration page (unsaved data will be lost)</a><hr></body></html>";
  server.send(200, "text/html", resp);
}

void writeStatusPage()
{
  char timeString[9];
  snprintf(timeString, sizeof(timeString)/sizeof(timeString[0]), "%02d:%02d:%02d", ourTime.hours, ourTime.minutes, ourTime.seconds);
  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred status page</title></head>\r\n"
              + "<body><h1>wiFred status</h1>\r\n"
              + "<table border=0>"
              + "<tr><td>Battery voltage: </td><td>" + batteryVoltage + " mV" + (lowBattery ? " Battery LOW" : "" ) + "</td></tr>\r\n"
              + "<tr><td>System time: </td><td>" + timeString + "</td></tr>\r\n";
  snprintf(timeString, sizeof(timeString)/sizeof(timeString[0]), "%02d:%02d:%02d", networkTime.hours, networkTime.minutes, networkTime.seconds);
  resp        += String("<tr><td>Network time: </td><td>") + timeString + "</td></tr>\r\n"
              + "<tr><td>Clock rate: </td><td>" + ourTime.rate10 / 10.0 + "</td></tr>\r\n"
              + "<tr><td colspan=2><a href=\"/\">Back to main configuration page</a></td></tr></table>\r\n"
              + "</body></html>";
  server.send(200, "text/html", resp);
}

void restartESP()
{
  ESP.restart();
}

void initWiFi(void)
{
  server.on("/", writeMainPage);
  server.on("/clock.html", writeClockPage);
  server.on("/loco.html", writeLocoPage);
  server.on("/funcmap.html", writeFuncMapPage);
  server.on("/status.html", writeStatusPage);
  server.on("/restart.html", restartESP);
  server.onNotFound(writeMainPage);
  
  // start configuration webserver
  server.begin();
}
