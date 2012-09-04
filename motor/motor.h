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
	MOTOR1,
	MOTOR2,
	MOTOR_LAST
};
typedef enum eMotor motor_id_t;

void MOTOR_Init(void);
void MOTOR_Set(motor_id_t motor, reg_val_t value);
reg_val_t MOTOR_Get(motor_id_t motor);

#endif /* MOTOR_H_ */
