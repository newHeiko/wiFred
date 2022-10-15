/**
 * This file is part of the wiFred wireless model railroading throttle project
 * Copyright (C) 2022  Heiko Rosemann
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
 * This file provides keymasks for all the different keys and it's intended to be 
 * shared between AVR and ESP12. Note ESP12 needs #defines for PC0..3 and PD4..7
 *
 * The code for de-bouncing has originally been developed by Peter Dannegger
 * and can be found i.e. at https://www.avrfreaks.net/projects/efficient-key-debounce
 * This code has been extended to cover more than one input port and to include
 * key release detection.
 */

#ifndef _COMMONKEY_H_
#define _COMMONKEY_H_

#ifndef PC0
#error "Define PC0..3 and PD4..7 before including commonKey.h - either by including avr/io.h or manually"
#endif

#define KEY_F1      (1ul<<PC0)
#define KEY_F4      (1ul<<PC1)
#define KEY_F7      (1ul<<PC2)
#define KEY_ESTOP   (1ul<<PC3)
#define KEY_F0      (1ul<<(PC0+8))
#define KEY_F2      (1ul<<(PC1+8))
#define KEY_F5      (1ul<<(PC2+8))
#define KEY_F8      (1ul<<(PC3+8))
#define KEY_F3      (1ul<<(PC0+16))
#define KEY_F6      (1ul<<(PC1+16))
#define KEY_SHIFT   (1ul<<(PC3+16))
#define KEY_REVERSE (1ul<<(PC1+24))
#define KEY_FORWARD (1ul<<(PC2+24))
#define KEY_LOCO1   (1ul<<PD7)
#define KEY_LOCO2   (1ul<<PD6)
#define KEY_LOCO3   (1ul<<PD5)
#define KEY_LOCO4   (1ul<<PD4)

#define KEY_ALL     KEY_FORWARD | KEY_REVERSE | KEY_ESTOP | KEY_SHIFT | KEY_F0 | KEY_F1 | KEY_F2 | KEY_F3 | KEY_F4 | KEY_F5 | KEY_F6 | KEY_F7 | KEY_F8 | KEY_LOCO1 | KEY_LOCO2 | KEY_LOCO3 | KEY_LOCO4

#endif
