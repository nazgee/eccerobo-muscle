/*
 * uart.h
 *
 *  Created on: Aug 12, 2012
 *      Author: nazgee
 */

#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>

void UART_Init(int bitrate);
void UART_TXbyte(char byte2tx);

#endif /* UART_H_ */
