/*
 * mypwm.c
 *
 *  Created on: Sep 11, 2012
 *      Author: nazgee
 */


#include "mypwm.h"



struct mypwm {
	uint8_t channels_number;
	pwm_desc_t channels[PWM_CHANNELS];
};

static struct mypwm this = {
		.channels_number = 0
};

void PWM_Init(void)
{

}

pwm_desc_ptr PWM_Register(void* userdata, state_handler_t onStart, state_handler_t onCycle, duty_t duty)
{
	// XXX assert that there are some channels left
	pwm_desc_ptr channel = &this.channels[this.channels_number];
	channel->userdata = userdata;
	channel->onStart = onStart;
	channel->onCycle = onCycle;

	PWM_Duty(channel, duty);

	this.channels_number++;

	return channel;
}

void PWM_Duty(pwm_desc_ptr channel, duty_t duty)
{
	channel->duty = duty;
}
