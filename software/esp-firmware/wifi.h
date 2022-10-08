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
#ifndef _WIFI_H_
#define _WIFI_H_

#include <vector>

#define SINGLE_NETWORK_TIMEOUT_MS 20000
#define TOTAL_NETWORK_TIMEOUT_MS 60000

#define UDP_BROADCAST_PORT 51289

typedef struct
{
  char * ssid;
  char * key;
  bool disabled = false;
} wifiAPEntry;

extern std::vector<wifiAPEntry> apList;

void initWiFi(void);

void initWiFiSTA(void);

void shutdownWiFiSTA(void);

void initMDNS(void);

void initWiFiAP(void);

void initWiFiConfigSTA(void);

void shutdownWiFiConfigSTA(void);

void handleWiFi(void);

void scanWifi(void);

void broadcastUDP(void);

#endif
