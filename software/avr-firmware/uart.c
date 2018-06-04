/**
 * This file is part of the wiFred wireless throttle project
 *
 * It provides functions for writing all key and speed information to the RS232 port and for reading LED values from RS232 port
 *
 * (c) 2018 Heiko Rosemann
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "led.h"
#include "timer.h"

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
 * Flag to show an entire line (ended in \n) has been received
 */
volatile bool rxDone = false;

/**
 * Indexes for UART TX queue
 */
volatile uint8_t readIndex = 0, writeIndex = 0;

/**
 * Enqueue data to be sent
 */
void uartSendData(char * data, uint8_t length)
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

/**
 * Handle fully received UART strings if there are any
 */
void uartHandler(void)
{
  if(!rxDone)
    {
      return;
    }

  char buffer[RX_BUFFER_SIZE];
  strncpy(buffer, rxBuffer, RX_BUFFER_SIZE);

  ledInfo temp;
  uint8_t led;

  if(sscanf_P(buffer, PSTR("L %hhu : %hhu/%hhu"),
	      &led, &temp.onTime, &temp.cycleTime) == 3)
    {
      if(led >= 1 && led <= 3 && temp.onTime <= temp.cycleTime)
	{
	  LEDs[led-1].onTime = temp.onTime;
	  LEDs[led-1].cycleTime = temp.cycleTime;
	}
      else
	{
	  uartSendData("Wrong parameters for LED settings",
		       sizeof("Wrong parameters for LED settings"));
	}
    }
  else if(buffer[0] == 'K')
    {
      keepaliveCountdownSeconds = SYSTEM_KEEPALIVE_TIMEOUT;
    }
  else
    {
      uartSendData("Unknown command", sizeof("Unknown command"));
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
