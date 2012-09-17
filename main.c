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

//void StartHandler ( void* userdata ) {
//	LED_On((uint8_t)userdata);
//}
//
//void CycleHandler ( void* userdata ) {
//	LED_Off((uint8_t)userdata);
//}
//
//pwm_desc_t pwm = {
//	.userdata = (void*)2,
//	.duty = 3000,
//	.onStart = StartHandler,
//	.onCycle = CycleHandler,
//};
int main(void)
{
	LED_Init();
	PWM_Init(PWM_PERIOD);
	MYMODBUS_Init(38400);

//	PWM_Register(&pwm);
	sei();

	while (1) {
		_delay_ms(1);
//		PWM_Duty(&pwm, pwm.duty+=30);
		LED_Off(LED_IDLE);
		MYMODBUS_Manage();
		LED_On(LED_IDLE);
	}
	return 0;
}
