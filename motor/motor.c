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


void pwm_OnDuty ( void* userdata );
void pwm_OnPeriodFinished ( void* userdata );

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
	struct motor motors[MOTORS_NUMBER];
};


#define SOFTSTART 75
struct module this = {
	.motors = {
		{
			.pin2 = {
				.pin = _BV(0),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pin1 = {
				.pin = _BV(1),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pwm = {
				.userdata = (void*)0,
				.duty_target = 0,
				.phase = 1,
				.duty_step = SOFTSTART,
				.onDuty = pwm_OnDuty,
				.onPeriodFinished = pwm_OnPeriodFinished,
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
				.duty_target = 0,
				.phase = 0,
				.duty_step = SOFTSTART,
				.onDuty = pwm_OnDuty,
				.onPeriodFinished = pwm_OnPeriodFinished,
			},
			.value = 0
		},
		{
			.pin2 = {
				.pin = _BV(4),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pin1 = {
				.pin = _BV(5),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pwm = {
				.userdata = (void*)2,
				.duty_target = 0,
				.phase = 1,
				.duty_step = SOFTSTART,
				.onDuty = pwm_OnDuty,
				.onPeriodFinished = pwm_OnPeriodFinished,
			},
			.value = 0
		},
		{
			.pin2 = {
				.pin = _BV(6),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pin1 = {
				.pin = _BV(7),
				.port = SFR2PTR(PORTB),
				.ddr = SFR2PTR(DDRB),
			},
			.pwm = {
				.userdata = (void*)3,
				.duty_target = 0,
				.phase = 0,
				.duty_step = SOFTSTART,
				.onDuty = pwm_OnDuty,
				.onPeriodFinished = pwm_OnPeriodFinished,
			},
			.value = 0
		}
	}
};

/*
 * This might not look obvious on first sight, but we are desperate to work in
 * slow decay mode, due to (pressumably) better efficiency.
 *
 * DRV8833 PWM controlling
 * IN1	IN2	FUNCTION
 * PWM	0	Forward PWM, fast decay
 * 1	PWM	Forward PWM, slow decay (better efficiency?)
 * 0	PWM	Reverse PWM, fast decay
 * PWM	1	Reverse PWM, slow decay (better efficiency?)
 */

void pwm_OnDuty ( void* userdata ) {
	struct motor* motor = &this.motors[(intptr_t)userdata];

	if (motor->value < 0) {
		*motor->pin2.port &= (~motor->pin2.pin);
	} else if (motor->value > 0){
		*motor->pin1.port &= (~motor->pin1.pin);
	}
}

void pwm_OnPeriodFinished ( void* userdata ) {
	struct motor* motor = &this.motors[(intptr_t)userdata];

	if (motor->value != 0) {
		*motor->pin2.port |= ( motor->pin2.pin);
		*motor->pin1.port |= ( motor->pin1.pin);
	} else {
		*motor->pin1.port &= (~motor->pin1.pin);
		*motor->pin2.port &= (~motor->pin2.pin);
	}
}

void MOTOR_Init(void) {
	for (uint8_t i=0; i<MOTORS_NUMBER; i++) {
		*this.motors[i].pin1.ddr |= this.motors[i].pin1.pin;
		*this.motors[i].pin2.ddr |= this.motors[i].pin2.pin;
		PWM_Register(&this.motors[i].pwm);
	}
}

static inline void motor_set(uint8_t id, reg_val_t value) {
	motor_value_t val_signed = value;

	if (val_signed < 0) {
		value = val_signed * -1;
	}

	struct motor* m = &this.motors[id];
	m->value = val_signed;
	PWM_Duty(&m->pwm, value / PWM_FACTOR);
}

void MOTOR_Set(motor_id_t motor, reg_val_t value) {
	switch(motor) {
	case MOTORS_LEFT:
		motor_set(MOTOR_LEFT_FRONT & MOTORS_ID_MASK, value);
		motor_set(MOTOR_LEFT_BACK & MOTORS_ID_MASK, value);
		break;
	case MOTORS_RIGHT:
		motor_set(MOTOR_RIGHT_FRONT & MOTORS_ID_MASK, value);
		motor_set(MOTOR_RIGHT_BACK & MOTORS_ID_MASK, value);
		break;
	case MOTORS_ALL:
		motor_set(MOTOR_LEFT_FRONT & MOTORS_ID_MASK, value);
		motor_set(MOTOR_LEFT_BACK & MOTORS_ID_MASK, value);
		motor_set(MOTOR_RIGHT_FRONT & MOTORS_ID_MASK, value);
		motor_set(MOTOR_RIGHT_BACK & MOTORS_ID_MASK, value);
		break;
	default:
		motor_set(motor & MOTORS_ID_MASK, value);
		break;
	}
}

//reg_val_t MOTOR_Get(motor_id_t motor) {
//
//}
