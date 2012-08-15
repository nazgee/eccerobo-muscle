/*
 * uart.c
 *
 *  Created on: Aug 12, 2012
 *      Author: nazgee
 */

#include "uart.h"

#define UART_PRESCALER(bitrate) (((F_CPU / (bitrate * 16UL))) - 1)

void UART_Init(int bitrate) {
	   UCSRB |= (1 << RXEN) | (1 << TXEN);   // Turn on the transmission and reception circuitry
	   UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes

	   UBRRH = (UART_PRESCALER(bitrate) >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	   UBRRL = UART_PRESCALER(bitrate); // Load lower 8-bits of the baud rate value into the low byte of the UBRR register

	   UCSRB |= (1 << RXCIE);
}

void UART_TXbyte(char byte2tx) {
	UDR = byte2tx;
}


ISR(USART_RXC_vect)
{
	char ReceivedByte;
	ReceivedByte = UDR; // Fetch the received byte value into the variable "ByteReceived"
	UDR = ++ReceivedByte; // Echo back the received byte back to the computer
}
