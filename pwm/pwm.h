/*
 * mypwm.h
 *
 *  Created on: Sep 11, 2012
 *      Author: nazgee
 */

#ifndef MYPWM_H_
#define MYPWM_H_


#include <stdint.h>
#include "../misc/list.h"

typedef void( *state_handler_t ) ( void* userdata );
typedef uint16_t duty_t;
typedef duty_t* duty_ptr;

typedef struct pwm_desc {
	struct list_head	node;
	void*			userdata;
	duty_t			duty;
	state_handler_t		onStart;
	state_handler_t		onCycle;
} pwm_desc_t;
typedef pwm_desc_t* pwm_desc_ptr;


void PWM_Init(duty_t period);
void PWM_Register(pwm_desc_ptr channel);
void PWM_Duty(pwm_desc_ptr channel, duty_t duty);



#endif /* MYPWM_H_ */
