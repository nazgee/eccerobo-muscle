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

#include "mymodbus.h"
#include "led.h"
#include "pwm/pwm.h"
#include "defines.h"
#include "motor/motor.h"


int main(void)
{
	LED_Init();
	PWM_Init(PWM_PERIOD);
	MYMODBUS_Init(38400);

	sei();

//	DDRB = 0xFF;
	while (1) {
		_delay_ms(2000);
		for (int i = 0; i < 100; ++i) {
			LED_On(LED_IDLE);
			MOTOR_Set(MOTORS_ALL, 15000);
			for (int j = 0; j <= i; ++j) {
				_delay_ms(200);
			}
			MOTOR_Set(MOTORS_ALL, 0);
			LED_Off(LED_IDLE);
			_delay_ms(1000);
		}
	}

	while (1) {
		_delay_ms(1);
		LED_Off(LED_IDLE);
		MYMODBUS_Manage();
		LED_On(LED_IDLE);
	}
	return 0;
}
