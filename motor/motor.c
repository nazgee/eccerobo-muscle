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

#define SFR2PTR(x) (uint8_t*)_SFR_ADDR(x)

struct motor_pin {
	volatile uint8_t* ddr;
	volatile uint8_t* port;
	uint8_t pin;
};

struct motor {
	struct motor_pin pin1;
	struct motor_pin pin2;
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

void MOTOR_Init(void) {
	for (uint8_t i=0; i<MOTOR_LAST; i++) {
		*this.motors[i].pin1.ddr |= this.motors[i].pin1.pin;
		*this.motors[i].pin2.ddr |= this.motors[i].pin2.pin;
	}
}

void MOTOR_Set(motor_id_t motor, reg_val_t value) {
	*this.motors[motor].pin1.port &= (~this.motors[motor].pin1.pin);
	*this.motors[motor].pin2.port &= (~this.motors[motor].pin2.pin);

	if (value == 1) {
		*this.motors[motor].pin1.port |= ( this.motors[motor].pin1.pin);
	} else if (value == 2){
		*this.motors[motor].pin2.port |= ( this.motors[motor].pin2.pin);
	}
}

//reg_val_t MOTOR_Get(motor_id_t motor) {
//
//}
