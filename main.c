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
	PWM_Init(PWM_PERIOD);

	sei();

//	PWM_Register((void*)_BV(0), StartHandler, CycleHandler, 2000);
//	PWM_Register((void*)_BV(1), StartHandler, CycleHandler, 2100);
//	PWM_Register((void*)_BV(2), StartHandler, CycleHandler, 2200);
//	PWM_Register((void*)_BV(3), StartHandler, CycleHandler, 2300);
//	PWM_Register((void*)_BV(4), StartHandler, CycleHandler, 10000);
//	PWM_Register((void*)_BV(5), StartHandler, CycleHandler, 20000);
//	PWM_Register((void*)_BV(6), StartHandler, CycleHandler, 30000);
	PWM_Register((void*)_BV(7), StartHandler, CycleHandler, 25000);
	PWM_QuickUpdate();

	while (1) {
//		_delay_ms(1);
		LED_On(LED_MODBUS);
		MYMODBUS_Manage();
		LED_Off(LED_MODBUS);
	}
	return 0;
}
