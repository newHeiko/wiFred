/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2018-2021 Heiko Rosemann
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
 * This file handles reading in keys and pushbuttons as well as setting LEDs
 * and reading in the speed potentiometer
 */

#ifndef _THROTTLE_HANDLING_H_
#define _THROTTLE_HANDLING_H_

#define LED_STOP 17
#define LED_FWD 19
#define LED_REV 18
#define FLASHLIGHT 20

enum keys { KEY_F0, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8,
            KEY_ESTOP, KEY_SHIFT, KEY_FWD, KEY_REV,
            KEY_LOCO1, KEY_LOCO2, KEY_LOCO3, KEY_LOCO4 };

const int KEY_PIN[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10,
            11, 12, 13, 14,
            15, 16, 17, 18 };

/**
 * Change LED settings
 */
void setLEDvalues(String ledFwd, String ledRev, String ledStop);

/**
 * Periodically check for new key settings
 */
void handleThrottle(void);

/**
 * Initialize key settings and LED timer settings
 */
void initThrottle(void);

#endif
