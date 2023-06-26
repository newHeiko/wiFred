/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2022 Heiko Rosemann
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
#include <ESPAsyncUDP.h>

#include "wifi.h"
#include "locoHandling.h"     // MODES, MODES_LENGTH
#include "config.h"
#include "lowbat.h"
#include "stateMachine.h"
#include "throttleHandling.h"
#include "gitVersion.h"

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
      dnsServer.processNextRequest(); // @suppress("No break at end of case")
      // intentional fall-through

    case STATE_CONNECTED:
    case STATE_LOCO_CONNECTING:
    case STATE_LOCO_WAITFORTIMEOUT:
    case STATE_LOCO_ONLINE:
    case STATE_LOCOS_OFF:
    case STATE_CONFIG_STATION:
    case STATE_CONFIG_STATION_WAITING:
      MDNS.update();
      break;

    case STATE_CONNECTING:
      wifiMulti.run();
      break;

    default:
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
  // count the number of available networks
  uint32_t numNetworks = 0;
  WiFi.mode(WIFI_STA);
  for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); it++)
  {
    if(!it->disabled)
    {
      wifiMulti.addAP(it->ssid, it->key);
      numNetworks++;
    }
  }

  // start configuration webserver
  server.begin();

  // no network found, quickly open config AP mode
  if(numNetworks == 0)
  {
    initWiFiAP();
    switchState(STATE_CONFIG_AP);
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
  String ssid = "wiFred-config" + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);
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

  // start configuration webserver
  server.begin();
}

void writeMainPage()
{
  uint8_t loco = server.arg("loco").toInt();

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
    locoServer.automatic = server.hasArg("loco.automatic");  //sloeber>> hasArg(String("loco.automatic")) makes sloeber happy

    if(!locoServer.automatic)
    {
      free(automaticServer);
      automaticServer = nullptr;
    }

    saveLocoServer();    
  }
  
  // check if this is a "set loco config" request (old format loco.address1)
  for(uint8_t i=0; i<4; i++)
  {
    if(server.hasArg (String("loco.address") + i+1) )
    {
      locos[i].address = server.arg(String("loco.address") + i+1).toInt();
      locos[i].mode = strdup(server.arg(String("loco.mode") + i+1).c_str());
      locos[i].longAddress = server.hasArg(String("loco.longAddress") + i+1);
      locos[i].direction = (eDirection) server.arg(String("loco.direction") + i+1).toInt();
      saveLocoConfig(i);
    }
  }

  // check if this is a "set loco config" request (new format loco and loco.address)
  if(loco >= 1 && loco <= 4 && server.hasArg("loco.address"))
  {
    locos[loco-1].address = server.arg("loco.address").toInt();
    locos[loco-1].mode = strdup(server.arg("loco.mode").c_str());
    locos[loco-1].longAddress = server.hasArg("loco.longAddress");
    locos[loco-1].direction = (eDirection) server.arg("loco.direction").toInt();
    saveLocoConfig(loco-1);
  }

  // check if this is a "set function configuration" request
  if(loco >= 1 && loco <= 4 && server.hasArg("f0") && server.hasArg("f1"))
  {
    for(uint8_t i=0; i<= MAX_FUNCTION; i++)
    {
      locos[loco-1].functions[i] = (functionInfo) server.arg(String("f") + i).toInt();
    }

    saveLocoConfig(loco-1);
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

  if (server.hasArg("centerSwitch"))
  {
    int temp = server.arg(String("centerSwitch")).toInt();
    if(CENTER_FUNCTION_IGNORE <= temp && temp <= MAX_FUNCTION)
    {
      centerFunction = temp;
      saveGeneralConfig();
    }
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
      if(server.hasArg("wifiKEY"))
      {
        newAP.key = strdup(server.arg("wifiKEY").c_str());
      }
      else
      {
        newAP.key = strdup("");
      }
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

  // check if this is a "disable WiFi network" request
  // will disable all networks with this SSID
  if (server.hasArg("disable"))
  {
    for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); ++it)
    {
      if(strcmp(it->ssid, server.arg("disable").c_str()) == 0)
      {
        it->disabled = true;
      }
    }
    saveWiFiConfig();
  }

  // check if this is an "enable WiFi network" request
  // will enable all networks with this SSID
  if (server.hasArg("enable"))
  {
    for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); ++it)
    {
      if(strcmp(it->ssid, server.arg("enable").c_str()) == 0)
      {
        it->disabled = false;
      }
    }
    saveWiFiConfig();
  }

  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html lang=\"en\"><head><meta charset=\"utf-8\"><title>wiFred configuration page</title></head>\r\n"
              + "<body><h1>wiFred configuration page</h1>\r\n"
              + "<hr>General configuration and status<hr>\r\n"
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Throttle name:</td><td><input type=\"text\" name=\"throttleName\" value=\"" + throttleName + "\"></td>"
              + "<td><input type=\"submit\" value=\"Save name\"></td></tr></table></form>\r\n"
              + "<table border=0>"
              + "<tr><td>Battery voltage: </td><td>" + batteryVoltage + " mV" + (lowBattery ? " Battery LOW" : "" ) + "</td></tr>"
              + "<tr><td>ESP Firmware revision: </td><td>" + REV + "</td></tr>"
	            + "<tr><td>AVR firmware revision: </td><td>" + avrRevision + "</td></tr></table>\r\n"
              + "<table><tr><td>Active WiFi network SSID:</td><td>" + (WiFi.isConnected() ? WiFi.SSID() : "not connected") + "</td></tr>"
              + "<tr><td>Signal strength:</td><td>" + (WiFi.isConnected() ? (String) WiFi.RSSI() + "dB" : "not connected") + "</td></tr>"
              + "<tr><td>WiFi STA MAC address:</td><td>" + WiFi.macAddress() + "</td></tr>"
              + "<tr><td colspan = 2><a href=\"./flashred.html\">Flash red LED to identify wiFred</a></td></tr></table>";
  
  for(uint8_t i=0; i<4; i++)
  {
    resp      += String("<hr>Loco configuration for loco: ") + (i+1) + "\r\n" 
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>DCC address: (-1 to disable)</td> <td><input type=\"text\" name=\"loco.address\" value=\"" + locos[i].address + "\"></td></tr>"
              + "<tr><td>Speed Step Mode: </td> <td><select name='loco.mode'>";
    for(int j=0; j<MODES_LENGTH; ++j)
    {
      resp += String("<option value=\"") + MODES[j].val + "\"";
      if( strcmp(MODES[j].val, locos[i].mode) == 0 )
      {
         resp += String(" selected");
      }
      resp += String(">") + MODES[j].val + " - "+ MODES[j].text + "</option>";
    }
    resp      += String("</select></td></tr>")
              + "<tr><td>Direction:</td><td>"
              + "<input type=\"radio\" name=\"loco.direction\" value=\"" + DIR_NORMAL + "\"" + (locos[i].direction == DIR_NORMAL ? " checked" : "" ) + ">Forward"
              + "<input type=\"radio\" name=\"loco.direction\" value=\"" + DIR_REVERSE + "\"" + (locos[i].direction == DIR_REVERSE ? " checked" : "" ) + ">Reverse"
              + "<input type=\"radio\" name=\"loco.direction\" value=\"" + DIR_DONTCHANGE + "\"" + (locos[i].direction == DIR_DONTCHANGE ? " checked" : "" ) + ">Don't change"
              + "</td></tr>"
              + "<tr><td>Long Address?</td> <td><input type=\"checkbox\" name=\"loco.longAddress\"" + (locos[i].longAddress ? " checked" : "" ) + "></td></tr>"
              + "<tr><td colspan=2><a href=\"funcmap.html?loco=" + (i+1) + "\">Function mapping</a></td></tr></table>"
              + "<input type=\"hidden\" name=\"loco\" value=\"" + (i+1) + "\"><input type=\"submit\" value=\"Save loco config\"></form>";
  }

  uint32_t numNetworks = 0;

  resp        += String("<hr>WiFi configuration<hr>\r\n")
              + "<table border=0><tr><td colspan=3><a href=scanWifi.html>Scan for networks</a></td></tr>"
              + "<tr><td colspan=3>Known and enabled WiFi networks:</td></tr>";
  for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); ++it)
  {
    if(!it->disabled)
    {
      resp += String("<tr><td>Network name: ") + it->ssid + "</td>"
          + "<td><form action=\"index.html\" method=\"get\"><input type=\"hidden\" name=\"remove\" value=\"" + it->ssid + "\"><input type=\"submit\" value=\"Remove Network\"></form></td>"
          + "<td><form action=\"index.html\" method=\"get\"><input type=\"hidden\" name=\"disable\" value=\"" + it->ssid + "\"><input type=\"submit\" value=\"Disable Network\"></form></td></tr>\r\n";
      numNetworks++;
    }
  }

  if(numNetworks == 0)
  {
    resp += String("<tr><td colspan=3>None.</td></tr>");
  }
  numNetworks = 0;

  resp += String("<tr><td colspan=3>Known but disabled WiFi networks:</td></tr>");

  for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); ++it)
  {
    if(it->disabled)
    {
      resp += String("<tr><td>Network: ") + it->ssid + "</td>"
          + "<td><form action=\"index.html\" method=\"get\"><input type=\"hidden\" name=\"remove\" value=\"" + it->ssid + "\"><input type=\"submit\" value=\"Remove Network\"></form></td>"
          + "<td><form action=\"index.html\" method=\"get\"><input type=\"hidden\" name=\"enable\" value=\"" + it->ssid + "\"><input type=\"submit\" value=\"Enable Network\"></form></td></tr>\r\n";
      numNetworks++;          
    }
  }

  if(numNetworks == 0)
  {
    resp += String("<tr><td colspan=3>None.</td></tr>");
  }
  
  resp        += String("<form action=\"index.html\" method=\"get\"><tr>")
                 + "<td>New SSID: <input type=\"text\" name=\"wifiSSID\"></td>"
                 + "<td>New PSK: <input type=\"text\" name=\"wifiKEY\"></td>"
                 + "<td><input type = \"submit\" value=\"Manually add network\"></td>"
                 +  "</tr></form></table>\r\n";

  resp        += String("<a href=restart.html>Restart wiFred to enable new WiFi settings</a>\r\n")
                 + " WiFi settings will not be active until restart.\r\n";
              
  resp        += String("<hr>Loco server configuration<hr>\r\n")
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Loco server and port: </td>"
              + "<td><input type=\"text\" name=\"loco.serverName\" value=\"" + locoServer.name + "\">:<input type=\"text\" name=\"loco.serverPort\" value=\"" + locoServer.port + "\"></td></tr>"
              + "<tr><td style=\"text-align: right\"><input type=\"checkbox\" name=\"loco.automatic\"" + (locoServer.automatic ? " checked" : "") + "></td><td>Find server automatically through Zeroconf/Bonjour instead.</td></tr>"
              + "<tr><td colspan=2>Using " + (locoServer.automatic && automaticServer != nullptr ? automaticServer : locoServer.name) + ":" + locoServer.port + "</td></tr>"
              + "<tr><td colspan=2><input type=\"submit\" value=\"Save loco server settings\"></td></tr></table></form>";

  resp        += String("<hr>wiFred system<hr>\r\n")
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr><td>Center position of direction switch:</td><td><select id=\"centerSwitch\" name=\"centerSwitch\">"
              + "<option value=\"-2\"" + (centerFunction == -2 ? " selected" : "") + ">No action</option>"
              + "<option value=\"-1\"" + (centerFunction == -1 ? " selected" : "") + ">Zero speed</option>";
  
  for(int f=0; f <= MAX_FUNCTION; f++)
  {
    resp += String("<option value=\"") + f + "\"" + (centerFunction == f ? " selected" : "") + ">Set F" + f + "</option>";
  }
              
  resp        += String("</select><input type=\"submit\" value=\"Save setting\"></td></tr></table></form>")
              + "<a href=resetConfig.html>Reset wiFred to factory defaults</a>\r\n"
              + "<a href=update>Update wiFred firmware</a>\r\n"
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

    saveLocoConfig(loco-1);
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
              + "<form action=\"index.html\" method=\"get\"><table border=0>"
              + "<tr style=\"text-align: center\">"
              + "<td>Function</td>"
              + "<td>Throttle controlled</td>"
              + "<td>Throttle controlled, force momentary</td>"
              + "<td>Throttle controlled, force locking</td>"
              + "<td>Throttle controlled if this is the only loco</td>"
              + "<td>Force function always on</td>"
              + "<td>Force function always off</td>"
              + "<td>Ignore function key</td></tr>";

    for(uint8_t i=0; i<=MAX_FUNCTION; i++)
    {
      resp    += String("<tr style=\"text-align: center");
      if(i%2)
      {
        resp  += String("; background-color: #eee");
      }
      resp += String("\"><td>F") + i + "</td>";
      for(uint8_t j=THROTTLE; j<=IGNORE; j++)
      {
        resp += String("<td><input type=\"radio\" name=\"f") + i + "\" value=\"" + j + "\"" 
             + (locos[loco-1].functions[i] == j ? " checked" : "" ) + "></td>";
      }
      resp    += String("</tr>");
    }
    resp      += String("<tr><td colspan=4><input type=\"hidden\" name=\"loco\" value=\"") + loco + "\"><input type=\"submit\" value=\"Save function configuration and return to main page\"></td></tr></table></form>\r\n";
  }
  resp        += String("<hr><a href=\"/\">Back to main configuration page (unsaved data will be lost)</a><hr></body></html>");
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
            + "<td>" + (WiFi.encryptionType(i) == ENC_TYPE_NONE ? "Unencrypted network" : "PSK: <input type=\"text\" name=\"wifiKEY\">") + "</td>"
            + "<td><input type=\"submit\" value=\"Add network\"></td>"
            + "<td>Signal strength: " + WiFi.RSSI(i) + "dB</td></tr></form>";
  }

  resp += String("</table>")
          + "<a href=\"/index.html\">Return to main page</a></body></html>";

  server.send(200, "text/html", resp);
}

void restartESP()
{
  String resp = String("<!DOCTYPE HTML>\r\n")
                + "<html><head><title>Restarting wiFred</title></head>\r\n"
                + "<body><h1>Restarting wiFred</h1>\r\n"
                + "<a href=\"/index.html\">Return to main page</a> (Might require reconnecting to wiFred WiFi)\r\n"
                + "</body></html>";
  server.send(200, "text/html", resp);
  delay(500);
  ESP.restart();
}

void resetESP()
{
  if(server.hasArg("reallyReset"))
  {
    String resp = String("<!DOCTYPE HTML>\r\n")
                  + "<html><head><title>Resetting wiFred to factory defaults</title></head>\r\n"
                  + "<body><h1>This wiFred is being reset to factory defaults</h1>\r\n"
                  + "<a href=\"/index.html\">Return to main page</a> (Might require reconnecting to wiFred WiFi)\r\n"
                  + "</body></html>";
    server.send(200, "text/html", resp);    
    deleteAllConfig();
    delay(500);
    restartESP();
  }
  else
  {
    String resp = String("<!DOCTYPE HTML>\r\n")
                  + "<html><head><title>Reset wiFred to factory defaults</title></head>\r\n"
                  + "<body><h1>Reset wiFred to factory defaults?</h1>\r\n"
                  + "<table border=0>"
                  + "<tr><td><a href=\"/resetConfig.html?reallyReset=on\">Yes, really reset the wiFred to factory defaults</a></td></tr>\r\n"
                  + "<tr><td><a href=\"/index.html\">No, return to configuration page</a></td></tr>\r\n"
                  + "</table>\r\n"
                  + "</body></html>";
    server.send(200, "text/html", resp);    
  }
}

/* db211109 begin
   return wiFred Config as XML-Data for using with Application as api */
void getConfigXML()
{
   /* get macadress */
   uint8_t mac[6];
   String macAdress = "";
   
  
     WiFi.macAddress(mac);
     macAdress = String(mac[0], 16) + String(mac[1], 16) + String(mac[2], 16) + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);
     
//     char * hostName = strdup(throttleName);

   /* collect response */      
   String resp = String("<?XML version=\"1.0\" encoding=\"UTF?8\"?>\r\n")
               
          + "<wiFred>\r\n"
                  + "<structurVersion value=\"1\"/>\r\n"
          
          + "<throttleName value=\"" + throttleName + "\"/>\r\n"
        //  + "<ownWifiName value=\"" + "? todo"+  "\"/>\r\n"  // name of the WiFi when device create one
        //  + "<computerName value=\"" + hostName + "\"/>\r\n" // name of the device shown in the network
          + "<localIP value=\"" + WiFi.localIP().toString() + "\"/>\r\n"
                  + "<firmwareRevision value=\"" + REV + "\"/>\r\n"

                  + "<batteryVoltage value=\""+ batteryVoltage+ "\"/>\r\n" 
          + "<batteryLow value=\"" + (lowBattery ? "1" : "0" )+ "\"/>\r\n"

          +"<WiFi>\r\n"
            + "  <Connected value=\"" + (WiFi.isConnected() ? "1" : "0" )+ "\"/>\r\n"
                    + "  <SSID value=\"" + (WiFi.isConnected() ? WiFi.SSID() : " " )+ "\"/>\r\n"
                    + "  <signalStrength value=\"" + (WiFi.isConnected() ?  (String) WiFi.RSSI() : " " )+ "\"/>\r\n"
                    + "  <macAdress value=\"" + macAdress + "\"/>\r\n"
                    
              +"</WiFi>\r\n";

          resp +="<LOCOS>\r\n";

          for(uint8_t i=0; i<4; i++)
          {
           resp      += " <LOCO ID=\"" + String(i+1) + "\">\r\n" 
              + "  <DCCadress value=\""+ locos[i].address + "\"/>\r\n"
              + "  <Mode value=\""+ locos[i].mode + "\" />\r\n"
              + "  <Direction value=\""+ locos[i].direction + "\" />\r\n"
              + "  <LongAdress value=\""+ locos[i].longAddress + "\" />\r\n"
              + "  <FUNCTIONS>\r\n" ; 

              for(uint8_t j=0; j<= MAX_FUNCTION; j++)
              {
                resp +=  "     <Function ID=\""+String(j)+"\" value=\""+ locos[i].functions[j] +"\" />\r\n" ;
              }

              resp +=  "  </FUNCTIONS>\r\n </LOCO>\r\n"; 
           }
           resp +="</LOCOS>\r\n";

           resp +="<NETWORKS>\r\n";
           for(std::vector<wifiAPEntry>::iterator it = apList.begin() ; it != apList.end(); ++it)
           {
                resp += String(" <NETWORK>\r\n  <SSID value=\"") + it->ssid + "\"/>\r\n";
                resp += String("  <Key value=\"") + it->key + "\"/>\r\n";
                if(!it->disabled)
                {
                  resp += "  <Enabled value=\"1\" />\r\n";     
                }
                else
                {
                  resp += "  <Enabled value=\"0\" />\r\n"; 
                }
                resp += " </NETWORK>\r\n";
                
           }
           resp +="</NETWORKS>\r\n";

           resp += "<LOCOSERVER>\r\n";
            resp += "   <ServerName value=\""  + String(locoServer.name) + "\" />\r\n";
            resp += "   <Port value=\"" + String(locoServer.port) + "\" />\r\n";
            resp += "   <Automatic value=\"" + String(locoServer.automatic) +"\" />\r\n";
           resp +="</LOCOSERVER>\r\n";

           resp += "<centerSwitch value=\"" + String(centerFunction) + "\" />\r\n";
           
    resp      +=   "</wiFred>\r\n";
        
   server.send(200, "text/html", resp);      
}
/* end db211109*/

void doFlashRED(void)
{
  if(server.hasArg("count"))
  {
    setLEDblink(server.arg("count").toInt());
  }
  else  
  {
    setLEDblink(10);
  }
  String resp = String("<!DOCTYPE HTML>\r\n")
              + "<html><head><title>Blinking red LED</title></head>\r\n"
              + "<body><h1>Blinking red LED</h1>\r\n"
              + "Red LED will blink " + (server.hasArg("count") ? server.arg("count").toInt() : 10) + " times.\r\n"
              + "<a href=\"/index.html\">Return to main page</a> (will not stop blinking)\r\n"
              + "</body></html>";
  server.send(200, "text/html", resp);    
}

void broadcastUDP(void)
{
  AsyncUDP udp;
  udp.broadcastTo("wiFred",UDP_BROADCAST_PORT);
}

void initWiFi(void)
{
  server.on("/", writeMainPage);
  server.on("/funcmap.html", writeFuncMapPage);
  server.on("/scanWifi.html", scanWifi);
  server.on("/restart.html", restartESP);
  server.on("/resetConfig.html", resetESP);
  server.on("/api/getConfigXML", getConfigXML); // db211109 return config as xml
  server.on("/flashred.html", doFlashRED);  //db 220828 let red LED flash from extern x times
  server.onNotFound(writeMainPage);

  updater.setup(&server);
}
