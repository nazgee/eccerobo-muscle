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

void StartHandler ( void* userdata ) {
	LED_Off((uint8_t)userdata);
}

void CycleHandler ( void* userdata ) {
	LED_On((uint8_t)userdata);
}


int main(void)
{
	LED_Init();

	MYMODBUS_Init(38400);

	PWM_Init(25000);
	PWM_Register((void*)_BV(0), StartHandler, CycleHandler, 2500);
	PWM_Register((void*)_BV(1), StartHandler, CycleHandler, 5000);
	PWM_Register((void*)_BV(2), StartHandler, CycleHandler, 10000);
	PWM_Register((void*)_BV(3), StartHandler, CycleHandler, 15000);
	PWM_Register((void*)_BV(4), StartHandler, CycleHandler, 20000);
	PWM_Register((void*)_BV(5), StartHandler, CycleHandler, 22500);
	PWM_Register((void*)_BV(6), StartHandler, CycleHandler, 24000);
	PWM_Register((void*)_BV(7), StartHandler, CycleHandler, 24900);
	PWM_Update();

//	int skipper = 0;
	while (1) {
//		skipper++;
//		if (skipper % 1000 == 0)
//			LED_Toggle(LED_IDLE);
		MYMODBUS_Manage();
	}
	return 0;
}
