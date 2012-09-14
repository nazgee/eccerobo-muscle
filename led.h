/*
 * uart.h
 *
 *  Created on: Aug 12, 2012
 *      Author: nazgee
 */

#ifndef LED_H_
#define LED_H_

#include <stdint.h>

#define LED_DDR DDRA
#define LED_PORT PORTA
#define LED_DUMMY7 _BV(7)
#define LED_DUMMY6 _BV(6)
#define LED_DUMMY5 _BV(5)
#define LED_DUMMY4 _BV(4)
#define LED_DUMMY3 _BV(3)
#define LED_MODBUS _BV(2)
#define LED_IRQ_ENABLED _BV(1)
#define LED_IDLE _BV(0)

void LED_Init(void);
void LED_On(uint8_t led);
void LED_Off(uint8_t led);
void LED_Toggle(uint8_t led);

#endif /* LED_H_ */
