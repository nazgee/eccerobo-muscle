/*
 * main.c
 *
 *  Created on: Aug 2, 2012
 *      Author: nazgee
 */

//#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"
#include "mymodbus.h"
#include "led.h"

int main(void)
{
	LED_Init();

	MYMODBUS_Init(4800);

	int skipper = 0;
	while (1) {
		skipper++;
		if (skipper % 1000 == 0)
			LED_Toggle(LED_IDLE);
		MYMODBUS_Manage();
	}
	return 0;
}
