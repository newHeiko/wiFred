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

#define SSID_CHARS 21
#define KEY_CHARS 21

typedef struct
{
  char ssid[SSID_CHARS];
  char key[KEY_CHARS];
} t_wlan;

extern t_wlan wlan;

void initWiFi(void);

void initWiFiSTA(void);

void shutdownWiFiSTA(void);

void initWiFiAP(void);

void initWiFiConfigSTA(void);

void shutdownWiFiConfigSTA(void);

void handleWiFi(void);

#endif

