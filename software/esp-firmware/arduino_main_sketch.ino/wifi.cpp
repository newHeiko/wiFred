/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018  Heiko Rosemann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 *
 * This file handles connecting and reconnecting to a wireless network as well
 * as providing a webserver for configuration of the device and status readout.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>

#include "wifi.h"
#include "locoHandling.h"
#include "config.h"
#include "lowbat.h"
#include "Ticker.h"
#include "stateMachine.h"
#include "throttleHandling.h"

// #define DEBUG

std::vector<wifiAPEntry> apList;

ESP8266WiFiMulti wifiMulti;

ESP8266WebServer server(80);
DNSServer dnsServer;
ESP8266HTTPUpdateServer updater;

void readString(char * dest, size_t maxLength, String input)
{
  strncpy(dest, input.c_str(), maxLength - 1);
  dest[maxLength - 1] = '\0';
}

void handleWiFi(void)
{
  server.handleClient();
  switch(wiFredState)
  {
    case STATE_CONFIG_AP:
      dnsServer.processNextRequest();
      // intentional fall-through

    case STATE_CONNECTED:
    case STATE_LOCO_CONNECTING:
    case STATE_LOCO_ONLINE:
    case STATE_CONFIG_STATION:
    case STATE_CONFIG_STATION_WAITING:
      MDNS.update();
      break;

    case STATE_LOWPOWER_WAITING:
    case STATE_LOWPOWER:
    case STATE_STARTUP:
    case STATE_CONNECTING:
      wifiMulti.run();
      break;
  }
}

/**
 * Stop listening on <throttleName>.local, start listening on config.local
 */
void initWiFiConfigSTA(void)
{
  MDNS.removeService(NULL, "http", "tcp");
  MDNS.setHostname("config");
  MDNS.addService("http", "tcp", 80);
}

/**
 * Stop listening on (MDNS) config.local, start listening on <throttleName>.local
 */
void shutdownWiFiConfigSTA(void)
{
  char * hostName = strdup(throttleName);
  
  for(char * src = throttleName, * dst = hostName; *src != 0;)
    {
      if(isalnum(*src))
      {
        *dst++ = *src++;
      }
      else
      {
        src++;
      }
      *dst = 0;
    }

#ifdef DEBUG
  Serial.println(String("Add MDNS ") + hostName + " on throttle name " + throttleName);
#endif

  MDNS.removeService(NULL, "http", "tcp");
  MDNS.setHostname(hostName);
  MDNS.addService("http", "tcp", 80);
}

void initWiFiSTA(void)
{
  WiFi.mode(WIFI_STA);
  for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); it++)
  {
    wifiMulti.addAP(it->ssid, it->key);
  }
}

void shutdownWiFiSTA(void)
{
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void initMDNS(void)
{
  char * hostName = strdup(throttleName);
  
  for(char * src = throttleName, * dst = hostName; *src != 0;)
    {
      if(isalnum(*src))
      {
        *dst++ = *src++;
      }
      else
      {
        src++;
      }
      *dst = 0;
    }

#ifdef DEBUG
  Serial.println(String("Add MDNS ") + hostName + " on throttle name " + throttleName);
#endif

  MDNS.begin(hostName);
  MDNS.addService("http", "tcp", 80);
}

void initWiFiAP(void)
{
  // open an AP for configuration if connection failed
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String ssid = "wiFred-config" + String(mac[0], 16) + String(mac[5], 16);
  #ifdef DEBUG
  Serial.println();
  Serial.print("Not connected, setting up AP at ");
  Serial.println(ssid);
  #endif
  WiFi.softAP(ssid.c_str());

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());

  MDNS.begin("config");
  MDNS.addService("http", "tcp", 80);
}

void writeMainPage()
{
  if(wiFredState == STATE_CONFIG_STATION_WAITING)
  {
    switchState(STATE_CONFIG_STATION);
    setLEDvalues("100/100", "100/100", "100/100");
  }

  // check if this is a "set loco server" request
  if(server.hasArg("loco.serverName") && server.hasArg("loco.serverPort"))
  {
    free(locoServer.name);
    locoServer.name = strdup(server.arg("loco.serverName").c_str());
    locoServer.port = server.arg("loco.serverPort").toInt();
    locoServer.automatic = server.hasArg("loco.automatic");

    if(!locoServer.automatic)
    {
      free(automaticServer);
      automaticServer = nullptr;
    }

    saveLocoServer();    
  }
  
  // check if this is a "set loco config" request
  #warning "TODO"
  {  
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
      
  // check if this is a "set general configuration" request
  if (server.hasArg("throttleName"))
  {
    free(throttleName);
    throttleName = strdup(server.arg("throttleName").c_str());
#ifdef DEBUG
    Serial.print("New throttleName: ");
    Serial.println(throttleName);
#endif
    saveGeneralConfig();
  }

  // check if this is a "manually add WiFi network" request
  if (server.hasArg("wifiSSID"))
  {
    wifiAPEntry newAP;

    newAP.ssid = strdup(server.arg("wifiSSID").c_str());
    if(strcmp(newAP.ssid, "") == 0)
    {
      free(newAP.ssid);
    }
    else
    {
      newAP.key = strdup(server.arg("wifiKEY").c_str());
      apList.push_back(newAP);

      saveWiFiConfig();
    }
  }
  
  // check if this is a "remove WiFi network" request
  // will remove all networks with this SSID
  if (server.hasArg("remove"))
  {
    for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); )
    {
      if(strcmp(it->ssid, server.arg("remove").c_str()) == 0)
      {
        free(it->ssid);
        free(it->key);
        it = apList.erase(it);
      }
      else
      {
        it++;
      }
    }
    saveWiFiConfig();
  }

  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred configuration page</title></head>\r\n"
              + "<body><h1>General configuration</h1>\r\n"
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Throttle name (max " + String(sizeof(throttleName)/sizeof(throttleName[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"throttleName\" value=\"" + throttleName + "\"></td></tr>"
              + "<tr><td>WiFi SSID (max " + String(sizeof(wlan.ssid)/sizeof(wlan.ssid[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"wifi.ssid\" value=\"" + wlan.ssid + "\"></td></tr>"
              + "<tr><td>WiFi PSK (max " + String(sizeof(wlan.key)/sizeof(wlan.key[0]) - 1) + " chars):</td><td><input type=\"text\" name=\"wifi.key\" value=\"" + wlan.key + "\"></td></tr>"
              + "<tr><td colspan=2><input type=\"submit\"></td></tr></table></form>\r\n";
  resp        += String("<hr>Loco configuration<hr>\r\n")
              + "<a href=loco.html>Loco configuration subpage</a>\r\n";
  resp        += String("<hr>Restart system to enable new WiFi settings<hr>\r\n")
              + "<a href=restart.html>Restart system to enable new WiFi settings</a>\r\n"
              + "<hr><hr>Status page<hr>\r\n"
              + "<a href=status.html>wiFred status subpage</a>\r\n"
              + "<hr><hr>Update firmware<hr>\r\n"
              + "<a href=update>Update wiFred firmware</a>\r\n"
              + "</body></html>";
  server.send(200, "text/html", resp);

  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred configuration page</title></head>\r\n"
              + "<body><h1>Loco configuration</h1>\r\n"
              + "<form action=\"loco.html\" method=\"get\"><table border=0>"
              + "<tr><td>Loco server and port: </td>"
              + "<td><input type=\"text\" name=\"loco.serverName\" value=\"" + locoServer.name + "\">:<input type=\"text\" name=\"loco.serverPort\" value=\"" + locoServer.port + "\"></td></tr>"
              + "<tr><td colspan=2><hr></td></tr>";
  for(uint8_t i=0; i<4; i++)
  {
    resp      += String("<tr><td>Loco ") + (i+1) + " DCC address: (-1 to disable)</td><td><input type=\"text\" name=\"loco.address" + (i+1) + "\" value=\"" + locos[i].address + "\">"
              + "Long Address? <input type=\"checkbox\" name=\"loco.longAddress" + (i+1) + "\"" + (locos[i].longAddress ? " checked" : "" ) + "></td></tr>"
              + "<tr><td>Reverse? <input type=\"checkbox\" name=\"loco.reverse" + (i+1) + "\"" + (locos[i].reverse ? " checked" : "" ) + "></td>"
              + "<td><a href=\"funcmap.html?loco=" + (i+1) + "\">Function mapping</a></td></tr>"
              + "<tr><td colspan=2><hr></td></tr>";
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

    saveLocoConfig(loco - 1);
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
  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>wiFred status page</title></head>\r\n"
              + "<body><h1>wiFred status</h1>\r\n"
              + "<table border=0>"
              + "<tr><td>Battery voltage: </td><td>" + batteryVoltage + " mV" + (lowBattery ? " Battery LOW" : "" ) + "</td></tr>\r\n"
              + "<tr><td colspan=2><a href=\"/\">Back to main configuration page</a></td></tr></table>\r\n"
              + "</body></html>";

  String resp = String("<!DOCTYPE HTML>\r\n")
                + "<html><head><title>wiClock configuration page</title></head>\r\n"
                + "<body><h1>wiClock configuration page</h1>\r\n"
                + "<hr>General configuration<hr>\r\n"
                + "<form action=\"index.html\" method=\"get\"><table border=0>"
                + "<tr><td>Throttle name:</td><td><input type=\"text\" name=\"throttleName\" value=\"" + throttleName + "\"></td></tr>"
                + "<tr><td colspan=2><input type=\"submit\" value=\"Save name\"></td></tr></table></form>\r\n"
                + "<hr>WiFi configuration<hr>\r\n"
                + "<table border=0><tr><td>Active WiFi network SSID:</td><td>" + (WiFi.isConnected() ? WiFi.SSID() : "not connected") + "</td><td><a href=scanWifi.html>Scan for networks</a></td></tr>"
                + "<tr><td colspan=3>Known WiFi networks:</td></tr>";
  for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); ++it)
  {
    resp += String("<tr><td>SSID: ") + it->ssid + "</td><td>PSK: " + it->key + "</td>"
          + "<td><form action=\"index.html\" method=\"get\"><input type=\"hidden\" name=\"remove\" value=\"" + it->ssid + "\"><input type=\"submit\" value=\"Remove SSID\"></form></td></tr>\r\n";
  }

  resp        += String("<form action=\"index.html\" method=\"get\"><tr>")
                 + "<td>New SSID: <input type=\"text\" name=\"wifiSSID\"></td>"
                 + "<td>New PSK: <input type=\"text\" name=\"wifiKEY\"></td>"
                 + "<td><input type = \"submit\" value=\"Manually add network\"></td>"
                 +  "</tr></form></table>\r\n";

  resp        += String("<a href=restart.html>Restart wiClock to enable new WiFi settings</a>\r\n")
                 + " WiFi settings will not be active until restart.\r\n";

  snprintf(timeString, sizeof(timeString) / sizeof(timeString[0]), "%02d:%02d:%02d", startupTime.hours, startupTime.minutes, startupTime.seconds);

  resp        += String("<hr>Clock configuration<hr>")
                 + "<table border=0><form action=\"index.html\" method=\"get\">"
                 + "<tr><td>Clock server and port: </td>"
                 + "<td>http://<input type=\"text\" name=\"clock.serverName\" value=\"" + clockServer.name + "\">:<input type=\"text\" name=\"clock.serverPort\" value=\"" + clockServer.port + "\">/json/time</td></tr>"
                 + "<tr><td>Find server automatically through Zeroconf/Bonjour?</td><td><input type=\"checkbox\" name=\"clock.automatic\"" + (clockServer.automatic ? " checked" : "") + ">"
                 + "Using http://" + (clockServer.automatic && automaticServer != nullptr ? automaticServer : clockServer.name) + ":" + clockServer.port + "/json/time</td></tr>"
                 + "<tr><td colspan=2><input type=\"submit\" value=\"Save clock server settings\"</td></tr></form>"
                 + "<form action=\"index.html\" method=\"get\">"
                 + "<tr><td>Startup time (format: H:M:S):</td><td><input type=\"text\" name=\"clock.startUp\" value=\"" + timeString + "\"></td></tr>"
                 + "<tr><td>Startup clock rate:</td><td><input type=\"text\" name=\"clock.startupRate\" value=\"" + startupTime.rate10 / 10.0 + "\"></td></tr>"
                 + "<tr><td colspan=2><input type=\"submit\" value=\"Save startup time and rate\"</td></tr></form>"
                 + "<form action=\"index.html\" method=\"get\">"
                 + "<tr><td>Clock offset from UTC (hours):</td><td><input type=\"text\" name=\"clock.offset\" value=\"" + clockOffset + "\"></td></tr>"
                 + "<tr><td>Maximum clock rate:</td><td><input type=\"text\" name=\"clock.maxClockRate\" value=\"" + clockMaxRate + "\"></td></tr>"
                 + "<tr><td>Pulse length for clock (milliseconds):</td><td><input type=\"text\" name=\"clock.pulseLength\" value=\"" + clockPulseLength + "\"></td></tr>"
                 + "<tr><td conspan=2><input type=\"submit\" value=\"Save clock configuration\"></td></tr></table></form>\r\n";

  snprintf(timeString, sizeof(timeString) / sizeof(timeString[0]), "%02d:%02d:%02d", ourTime.hours, ourTime.minutes, ourTime.seconds);
  resp        += String("<hr>wiClock status<hr>\r\n")
                 + "<table border=0>"
                 + "<tr><td>Battery voltage: </td><td>" + batteryVoltage + " mV" + (lowBattery ? " Battery LOW" : "" ) + "</td></tr>\r\n"
                 + "<tr><td>System time: </td><td>" + timeString + "</td></tr>\r\n";
  
  snprintf(timeString, sizeof(timeString) / sizeof(timeString[0]), "%02d:%02d:%02d", networkTime.hours, networkTime.minutes, networkTime.seconds);
  resp        += String("<tr><td>Network time: </td><td>") + timeString + "</td></tr>\r\n"
                 + "<tr><td>Clock rate: </td><td>" + ourTime.rate10 / 10.0 + "</td></tr></table>\r\n"
                 + "<hr>wiClock system<hr>\r\n"
                 + "<a href=resetConfig.html>Reset wiClock to factory defaults</a>\r\n"
                 + "<a href=update>Update wiClock firmware</a>\r\n"
                 + "</body></html>";
  server.send(200, "text/html", resp);
}

void scanWifi()
{
  uint32_t n = WiFi.scanNetworks();
  
  String resp = String("<!DOCTYPE HTML>\r\n")
                + "<html><head><title>Scan for WiFi networks</title></head>"
                + "<body><h1>Results of WiFi scan</h1>"
                + "<table border=0>";

  if(n == 0)
  {
    resp += String("<tr><td>No WiFi networks found. Reload to repeat scan.</td></tr>");
  }
  
  for(uint32_t i = 0; i < n; i++)
  {
    resp += String("<form action=\"index.html\" method = \"get\">")
            + "<tr><td>" + WiFi.SSID(i) + "<input type=\"hidden\" name=\"wifiSSID\" value=\"" + WiFi.SSID(i) + "\"></td>"
            + "<td>Enter PSK here if required: <input type=\"text\" name=\"wifiKEY\"></td>"
            + "<td><input type=\"submit\" value=\"Add network\"></td></tr></form>";
  }

  resp += String("</table>")
          + "<a href=index.html>Return to main page</a></body></html>";

  server.send(200, "text/html", resp);
}

void restartESP()
{
  ESP.restart();
}

void resetESP()
{
  if(server.hasArg("reallyReset"))
  {
    deleteAllConfig();
    restartESP();
  }
  else
  {
    String resp = String("<!DOCTYPE HTML>\r\n")
                  + "<html><head><title>Reset wiFred to factory defaults</title></head>\r\n"
                  + "<body><h1>Reset wiFred to factory defaults?</h1>\r\n"
                  + "<table border=0>"
                  + "<tr><td><a href=\"/resetConfig?reallyReset=on\">Yes, really reset the wiFred to factory defaults</a></td></tr>\r\n"
                  + "<tr><td><a href=\"/index.html\">No, return to configuration page</a></td></tr>\r\n"
                  + "</table>\r\n"
                  + "</body></html>";
    server.send(200, "text/html", resp);    
  }
}

void initWiFi(void)
{
  server.on("/", writeMainPage);
  server.on("/funcmap.html", writeFuncMapPage);
  server.on("/scanWifi.html", scanWifi);
  server.on("/restart.html", restartESP);
  server.on("/resetConfig.html", resetESP);
  server.onNotFound(writeMainPage);

  updater.setup(&server);

  // start configuration webserver
  server.begin();
}
