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
#include "pwm/pwm.h"

int main(void)
{
	LED_Init();
	PWM_Init(PWM_PERIOD);
	MYMODBUS_Init(38400);

	sei();


	while (1) {
//		_delay_ms(1);
		LED_Off(LED_IDLE);
		MYMODBUS_Manage();
		LED_On(LED_IDLE);
	}
	return 0;
}
