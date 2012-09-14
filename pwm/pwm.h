/*
 * mypwm.h
 *
 *  Created on: Sep 11, 2012
 *      Author: nazgee
 */

#ifndef MYPWM_H_
#define MYPWM_H_


#include <stdint.h>

#define PWM_CHANNELS 10
#define PWM_PERIOD 32767

typedef void( *state_handler_t ) ( void* userdata );
typedef uint16_t duty_t;
typedef duty_t* duty_ptr;

typedef struct pwm_desc {
	void* userdata;
	duty_t duty;
	state_handler_t onStart;
	state_handler_t onCycle;
} pwm_desc_t;
typedef pwm_desc_t* pwm_desc_ptr;


void PWM_Init(duty_t period);
uint8_t PWM_Register(void* userdata, state_handler_t onStart, state_handler_t onCycle, duty_t duty);
void PWM_Duty(uint8_t id, duty_t duty);
void PWM_Update(void);
void PWM_QuickUpdate(void);



#endif /* MYPWM_H_ */
