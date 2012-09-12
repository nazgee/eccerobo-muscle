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
#include "pwm/mypwm.h"

void StartHandler ( void* userdata ) {
	LED_Off((uint8_t)userdata);
}

void CycleHandler ( void* userdata ) {
	LED_On((uint8_t)userdata);
}


int main(void)
{
	LED_Init();

	MYMODBUS_Init(4800);

	PWM_Init(2500);
	sei();
	TCCR1B = 2;
	PWM_Register((void*)_BV(0), StartHandler, CycleHandler, 250);
	PWM_Register((void*)_BV(1), StartHandler, CycleHandler, 500);
	PWM_Register((void*)_BV(2), StartHandler, CycleHandler, 1000);
	PWM_Register((void*)_BV(3), StartHandler, CycleHandler, 1500);
	PWM_Register((void*)_BV(4), StartHandler, CycleHandler, 2000);
	PWM_Register((void*)_BV(5), StartHandler, CycleHandler, 2250);
	PWM_Register((void*)_BV(6), StartHandler, CycleHandler, 2400);
	PWM_Register((void*)_BV(7), StartHandler, CycleHandler, 2490);
	PWM_Update();

	int skipper = 0;
	while (1) {
		skipper++;
//		if (skipper % 1000 == 0)
//			LED_Toggle(LED_IDLE);
		MYMODBUS_Manage();
	}
	return 0;
}
