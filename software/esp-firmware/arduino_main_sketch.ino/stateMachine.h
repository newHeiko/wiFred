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
 * This file provides definitions for the global state machine.
 */

#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include <stdint.h>

enum state { STATE_STARTUP, STATE_CONNECTING, STATE_CONNECTED, STATE_CONFIG_AP, STATE_CONFIG_STATION_WAITING, STATE_CONFIG_STATION_COMING, STATE_CONFIG_STATION, STATE_LOWPOWER_WAITING, STATE_LOWPOWER };

extern state wiFredState;

void switchState(state newState, uint32_t timeout = UINT32_MAX);

#endif
