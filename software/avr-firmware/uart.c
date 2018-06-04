/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for writing all key and speed information to the RS232 port and for reading LED values from RS232 port
 *
 * (c) 2018 Heiko Rosemann
 */

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

#define BAUD 115200
#include <util/setbaud.h>

/**
 * Initialize UART for 115200 8N1, RXC interrupt
 */
void initUART(void)
{
}

/**
 * Buffer for UART queues
 */
volatile char txBuffer[TX_BUFFER_SIZE];
volatile char rxBuffer[RX_BUFFER_SIZE];

volatile bool rxDone = false;

/**
 * Indexes for UART TX queue
 */
volatile uint8_t readIndex = 0, writeIndex = 0;

/**
 * Enqueue data to be sent
 */
void uartSendData(uint8_t * data, uint8_t length)
{
  while(length-- != 0)
    {
      txBuffer[writeIndex++] = *data++;
      if(writeIndex >= TX_BUFFER_SIZE)
	{
	  writeIndex = 0;
	}
    }

  // check if UART is active
  if(! (UCSR0B & (1<<UDRIE0)) )
    {
      // transmit first byte if not
      UCSR0B |= (1<<UDRIE0);
      UDR0 = txBuffer[readIndex++];
      if(readIndex >= TX_BUFFER_SIZE)
	{
	  readIndex = 0;
	}
    }
}

/**
 * UDR empty interrupt - send next byte, if expected
 */
ISR(USART_UDRE_vect)
{
  if(readIndex != writeIndex)
    {
      UDR0 = txBuffer[readIndex++];
      if(readIndex >= TX_BUFFER_SIZE)
	{
	  readIndex = 0;
	}
    }
  else
    {
      UCSR0B &= ~(1<<UDRIE0);
    }
}

ISR(USART_RX_vect)
{
  char data = UDR0;

  static uint8_t rx_in = 0;

  if(data == '\n' || data == '\r')
    {
      // check if any data has been received before
      if(rx_in != 0)
	{
	  // reception has been completed
	  rxDone = true;
	  if(rx_in < RX_BUFFER_SIZE)
	    {	      
	      rxBuffer[rx_in] = 0;
	    }
	  else
	    {
	      rxBuffer[RX_BUFFER_SIZE - 1] = 0;
	    }
	  rx_in = 0;
	}
    }
  else
    {
      // guard against buffer overflow
      if(rx_in < RX_BUFFER_SIZE)
	{
	  rxBuffer[rx_in] = data;
	}
      rx_in++;
    }
  
}
