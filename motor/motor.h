/*
 * motor.h
 *
 *  Created on: Sep 5, 2012
 *      Author: nazgee
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "../regs/reg.h"

enum eMotor{
	MOTORS_RIGHT = 0x80,
	MOTORS_LEFT = 0x40,
	MOTORS_ALL = MOTORS_RIGHT | MOTORS_LEFT,

	MOTORS_ID_MASK = 0x0F,

	MOTOR_RIGHT_FRONT = MOTORS_RIGHT | 0,
	MOTOR_RIGHT_BACK = MOTORS_RIGHT | 1,
	MOTOR_LEFT_FRONT = MOTORS_LEFT | 2,
	MOTOR_LEFT_BACK = MOTORS_LEFT | 3,

	MOTORS_NUMBER = 4
};
typedef enum eMotor motor_id_t;
typedef int16_t motor_value_t;

void MOTOR_Init(void);
void MOTOR_Set(motor_id_t motor, reg_val_t value);
reg_val_t MOTOR_Get(motor_id_t motor);

#endif /* MOTOR_H_ */
