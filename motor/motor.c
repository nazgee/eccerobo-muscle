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

#define SFR2PTR(x) (uint8_t*)_SFR_ADDR(x)

struct motor_pin {
	volatile uint8_t* ddr;
	volatile uint8_t* port;
	uint8_t pin;
};

struct motor {
	struct motor_pin pin1;
	struct motor_pin pin2;
	uint8_t pwm;
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
				.ddr = SFR2PTR(DDRD)
			},
			.pin2 = {
				.pin = _BV(5),
				.port = SFR2PTR(PORTD),
				.ddr = SFR2PTR(DDRD)
			}
		},
		{
			.pin1 = {
				.pin = _BV(2),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB)
			},
			.pin2 = {
				.pin = _BV(3),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB)
			}
		}
	}
};

void pwm_StartHandler ( void* userdata ) {
	struct motor* motor = (struct motor*)userdata;
	if (motor->value < 0) {
		*motor->pin1.port |= ( motor->pin1.pin);
	} else {
		*motor->pin2.port |= ( motor->pin2.pin);
	}

}

void pwm_CycleHandler ( void* userdata ) {
	struct motor* motor = (struct motor*)userdata;

	*motor->pin1.port &= (~motor->pin1.pin);
	*motor->pin2.port &= (~motor->pin2.pin);
}

void MOTOR_Init(void) {
	for (uint8_t i=0; i<MOTOR_LAST; i++) {
		*this.motors[i].pin1.ddr |= this.motors[i].pin1.pin;
		*this.motors[i].pin2.ddr |= this.motors[i].pin2.pin;
		this.motors[i].pwm = PWM_Register(&this.motors[i], pwm_StartHandler, pwm_CycleHandler, 1);
	}
}

void MOTOR_Set(motor_id_t motor, reg_val_t value) {
	motor_value_t val = value;

	if (val < 0) {
		val *= -1;
	}

	struct motor* m = &this.motors[motor];
	m->value = val;
	PWM_Duty(m->pwm, val);
	PWM_Update();
}

//reg_val_t MOTOR_Get(motor_id_t motor) {
//
//}
