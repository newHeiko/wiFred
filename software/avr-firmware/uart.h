/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for writing all key and speed information to the RS232 port and for reading LED values from RS232 port
 *
 * (c) 2018 Heiko Rosemann
 */

#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
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

#endif
