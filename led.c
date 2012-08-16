/*
 * uart.c
 *
 *  Created on: Aug 12, 2012
 *      Author: nazgee
 */
#include <avr/io.h>
#include "led.h"
#include <util/delay.h>

void LED_Init(void) {
	LED_DDR = 0xFF;
	for (int i=0; i<8; i++) {
		LED_On(_BV(i));
		_delay_ms(100);
	}
	LED_PORT = 0;
}

void LED_On(uint8_t led) {
	LED_PORT |= led;
}

void LED_Off(uint8_t led) {
	LED_PORT &= ~led;
}

void LED_Toggle(uint8_t led) {
	LED_PORT ^= led;
}

