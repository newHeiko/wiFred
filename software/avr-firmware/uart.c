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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "uart.h"
#include "led.h"
#include "timer.h"
#include "keypad.h"

#define BAUD 115200
#include <util/setbaud.h>

/**
 * Initialize UART for 115200 8N1, RXC interrupt
 */
void initUART(void)
{
  UCSR0A = (USE_2X<<U2X0);
  UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
}

/**
 * Buffer for UART queues
 */
volatile char txBuffer[TX_BUFFER_SIZE];
volatile char rxBuffer[RX_BUFFER_SIZE];

/**
 * Number of entire lines (ending in \n) that have been received
 */
volatile uint8_t rxDone = 0;

/**
 * Indexes for UART TX queue and RX queue
 */
volatile uint8_t txReadIndex = 0, txWriteIndex = 0;
volatile uint8_t rxWriteIndex = 0, rxReadIndex = 0;

/**
 * Enqueue speed data to be sent
 *
 * Returns true if sent
 *         false if UART busy
 */
bool uartSendSpeed(uint8_t speed)
{
  char buffer[sizeof("S:100:F\r\n")];
  if(! (UCSR0B & (1<<UDRIE0)) )
    {
      if(getKeyState(KEY_FORWARD))
	{
	  snprintf(buffer, sizeof("S:100:F\r\n"), "S:%03u:F\r\n", speed);
	}
      else if(getKeyState(KEY_REVERSE))
	{
	  snprintf(buffer, sizeof("S:100:R\r\n"), "S:%03u:R\r\n", speed);
	}
      uartSendData(buffer, sizeof("S:100:F\r\n") - 1);
      return true;
    }
  else
    {
      return false;
    }
}

/**
 * Enqueue data to be sent
 */
void uartSendData(char * data, uint8_t length)
{
  while(length-- != 0)
    {
      txBuffer[txWriteIndex++] = *data++;
      if(txWriteIndex >= TX_BUFFER_SIZE)
	{
	  txWriteIndex = 0;
	}
    }

  // check if UART is active
  if(! (UCSR0B & (1<<UDRIE0)) )
    {
      // transmit first byte if not
      UDR0 = txBuffer[txReadIndex++];
      if(txReadIndex >= TX_BUFFER_SIZE)
	{
	  txReadIndex = 0;
	}
      UCSR0B |= (1<<UDRIE0);
    }
}

/**
 * UDR empty interrupt - send next byte, if expected
 */
ISR(USART_UDRE_vect)
{
  if(txReadIndex != txWriteIndex)
    {
      UDR0 = txBuffer[txReadIndex++];
      if(txReadIndex >= TX_BUFFER_SIZE)
	{
	  txReadIndex = 0;
	}
    }
  else
    {
      UCSR0B &= ~(1<<UDRIE0);
    }
}

/**
 * Handle fully received UART strings if there are any
 */
void uartHandler(void)
{
  if(rxDone == 0)
    {
      return;
    }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    rxDone--;
  }

  char buffer[RX_BUFFER_SIZE];

  uint8_t index = 0;
  
  while(rxBuffer[rxReadIndex] != '\r' && rxBuffer[rxReadIndex] != '\n' && rxReadIndex != rxWriteIndex && index < RX_BUFFER_SIZE - 1)
    {
      buffer[index++] = rxBuffer[rxReadIndex++];
      if(rxReadIndex >= RX_BUFFER_SIZE)
	{
	  rxReadIndex = 0;
	}
    }

  // skip over delimiter
  rxReadIndex++;

  if(rxReadIndex >= RX_BUFFER_SIZE)
    {
      rxReadIndex = 0;
    }

  // make sure buffer is properly terminated
  buffer[index] = 0;

  ledInfo temp;
  uint8_t led;

  if(sscanf_P(buffer, PSTR("L%hhu: %hhu/%hhu"),
	      &led, &temp.onTime, &temp.cycleTime) == 3)
    {
      if(led >= 1 && led <= 3)
	{
	  if(LEDs[led-1].onTime != temp.onTime || LEDs[led-1].cycleTime != temp.cycleTime)
	    {
	      newLEDvalues();
	    }	      
	  LEDs[led-1].onTime = temp.onTime;
	  LEDs[led-1].cycleTime = temp.cycleTime;
	  uartSendData("LOK\r\n", sizeof("LOK\r\n") - 1);
	}
      else
	{
	  uartSendData("LERR\r\n", sizeof("LERR\r\n") - 1);
	}
    }
  else if(buffer[0] == 'K')
    {
      keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;
      uartSendData("KOK\r\n", sizeof("KOK\r\n") - 1);
    }
  else
    {
      uartSendData("ERR\r\n", sizeof("ERR\r\n") - 1);
    }
}


ISR(USART_RX_vect)
{
  char data = UDR0;

  if(data == '\n' || data == '\r')
    {
      // reception has been completed
      rxDone++;
    }

  // read data into buffer
  rxBuffer[rxWriteIndex++] = data;

  // wrap around if end of buffer
  if(rxWriteIndex >= RX_BUFFER_SIZE)
    {
      rxWriteIndex = 0;
    }  
}
