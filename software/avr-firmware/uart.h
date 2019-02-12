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
 * This file provides functions for writing all key and speed information to 
 * the RS232 port and for reading LED values from RS232 port
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

/**
 * Size of UART TX and RX buffers
 */
#define TX_BUFFER_SIZE 64
#define RX_BUFFER_SIZE 64

/**
 * Initialize UART for 115200 8N1, RXC interrupt
 */
void initUART(void);

/**
 * Enqueue data to be sent
 */
void uartSendData(char * data, uint8_t length);

/**
 * Handle fully received UART strings if there are any
 */
void uartHandler(void);

/**
 * This will be true after receiving "online" status from ESP8266
 *             false after receiving "offline" status from ESP8266
 *             false at startup of ESP8266
 */
extern volatile bool wifiOnline;

#endif
