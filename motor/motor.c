/*
 * motor.c
 *
 *  Created on: Sep 5, 2012
 *      Author: nazgee
 */

#include <avr/io.h>
#include <avr/portpins.h>
#include "motor.h"
#include "../led.h"
#include "../pwm/pwm.h"
#include "../defines.h"


void pwm_StartHandler ( void* userdata );
void pwm_CycleHandler ( void* userdata );

#define SFR2PTR(x) (uint8_t*)_SFR_ADDR(x)

struct motor_pin {
	volatile uint8_t* ddr;
	volatile uint8_t* port;
	uint8_t pin;
};

struct motor {
	struct motor_pin pin1;
	struct motor_pin pin2;
	pwm_desc_t pwm;
	motor_value_t value;
};

struct module {
	struct motor motors[MOTOR_LAST];
};



struct module this = {
	.motors = {
		{
			.pin1 = {
				.pin = _BV(4),
				.port = SFR2PTR(PORTD),
				.ddr = SFR2PTR(DDRD),
			},
			.pin2 = {
				.pin = _BV(5),
				.port = SFR2PTR(PORTD),
				.ddr = SFR2PTR(DDRD),
			},
			.pwm = {
				.userdata = (void*)0,
				.duty = 0,
				.onStart = pwm_StartHandler,
				.onCycle = pwm_CycleHandler,
			},
			.value = 0
		},
		{
			.pin2 = {
				.pin = _BV(2),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pin1 = {
				.pin = _BV(3),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pwm = {
				.userdata = (void*)1,
				.duty = 0,
				.onStart = pwm_StartHandler,
				.onCycle = pwm_CycleHandler,
			},
			.value = 0
		}
	}
};

void pwm_StartHandler ( void* userdata ) {
	struct motor* motor = &this.motors[(intptr_t)userdata];
	if (motor->value < 0) {
		*motor->pin1.port |= ( motor->pin1.pin);
	} else if (motor->value > 0){
		*motor->pin2.port |= ( motor->pin2.pin);
	}

}

void pwm_CycleHandler ( void* userdata ) {
	struct motor* motor = &this.motors[(intptr_t)userdata];

	*motor->pin1.port &= (~motor->pin1.pin);
	*motor->pin2.port &= (~motor->pin2.pin);
}

void MOTOR_Init(void) {
	for (uint8_t i=0; i<MOTOR_LAST; i++) {
		*this.motors[i].pin1.ddr |= this.motors[i].pin1.pin;
		*this.motors[i].pin2.ddr |= this.motors[i].pin2.pin;
		PWM_Register(&this.motors[i].pwm);
	}
}

void MOTOR_Set(motor_id_t motor, reg_val_t value) {
	motor_value_t val_signed = value;

	if (val_signed < 0) {
		value = val_signed * -1;
	}

	struct motor* m = &this.motors[motor];
	m->value = val_signed;
	PWM_Duty(&m->pwm, value / PWM_FACTOR);
}

//reg_val_t MOTOR_Get(motor_id_t motor) {
//
//}
