/*
 * mypwm.c
 *
 *  Created on: Sep 11, 2012
 *      Author: nazgee
 */


#include "mypwm.h"
#include "../led.h"

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define ENTER_CRIT() cli()
#define EXIT_CRIT() cli()


struct mypwm {
	duty_t period;
	uint8_t current_channel;

	volatile uint8_t dirty;
	uint8_t channels_number;
	uint8_t channels_number_dirty;

	pwm_desc_t channels_A[PWM_CHANNELS];
	pwm_desc_t channels_B[PWM_CHANNELS];
	pwm_desc_ptr channels;
	pwm_desc_ptr channels_dirty;

	duty_t deltas_A[PWM_CHANNELS + 1];
	duty_t deltas_B[PWM_CHANNELS + 1];
	duty_ptr deltas;
	duty_ptr deltas_dirty;
};

static struct mypwm this = {
	.channels_number = 0,
	.channels_number_dirty = 0,
	.current_channel = 0,
};

static inline void pwm_WaitIfDirty(void)
{
	while(this.dirty);
}

static inline void pwm_SwapBuffers(void)
{
	if (this.channels == this.channels_A) {
		this.channels = this.channels_B;
		this.channels_dirty = this.channels_A;
		this.deltas = this.deltas_B;
		this.deltas_dirty = this.deltas_A;
	} else {
		this.channels = this.channels_A;
		this.channels_dirty = this.channels_B;
		this.deltas = this.deltas_A;
		this.deltas_dirty = this.deltas_B;
	}

	this.channels_number = this.channels_number_dirty;
}

static int pwm_CompareDuties(const void *cmp1, const void *cmp2) {
	const pwm_desc_ptr pwm1 = (pwm_desc_ptr)cmp1;
	const pwm_desc_ptr pwm2 = (pwm_desc_ptr)cmp2;
	return pwm1->duty - pwm2->duty;
}

void PWM_Init(duty_t period)
{
	this.period = period;
	this.channels = this.channels_B;
	this.channels_dirty = this.channels_A;
	this.deltas = this.deltas_B;
	this.deltas_dirty = this.deltas_A;

	TIMSK |= _BV(OCIE1B);
}

pwm_desc_ptr PWM_Register(void* userdata, state_handler_t onStart, state_handler_t onCycle, duty_t duty)
{
	// XXX assert that there are some channels left for registration
	pwm_desc_ptr channel = &this.channels_dirty[this.channels_number_dirty];
	channel->userdata = userdata;
	channel->onStart = onStart;
	channel->onCycle = onCycle;

	PWM_Duty(channel, duty);

	this.channels_number_dirty++;

	return channel;
}

void PWM_Duty(pwm_desc_ptr channel, duty_t duty)
{
	channel->duty = duty;
}

void PWM_Update(void)
{
	qsort(this.channels_dirty, this.channels_number_dirty, sizeof(this.channels_dirty[0]), pwm_CompareDuties);
	duty_t prev_duty = 0;
	for (uint8_t i = 0; i < this.channels_number_dirty; i++) {
		this.deltas_dirty[i] = this.channels_dirty[i].duty - prev_duty;
		prev_duty = this.channels_dirty[i].duty;
	}
	this.deltas_dirty[this.channels_number_dirty] = this.period - prev_duty;
	this.dirty = 1;
	pwm_WaitIfDirty();
}

ISR( TIMER1_COMPB_vect)
{
	OCR1B += this.deltas[this.current_channel];
	LED_Toggle(LED_DUMMY4);

	if (this.current_channel < this.channels_number) {
//		LED_Toggle(LED_DUMMY3);
		do {
			pwm_desc_ptr channel = &this.channels[this.current_channel];
			channel->onCycle(channel->userdata);
		} while ((this.deltas[++this.current_channel] == 0)
				&& (this.current_channel < this.channels_number));
	} else {
//		LED_Toggle(LED_DUMMY2);
		while (this.current_channel > 0) {
			this.current_channel--;
			pwm_desc_ptr channel = &this.channels[this.current_channel];
			channel->onStart(channel->userdata);
		}

		if (this.dirty) {
			this.dirty = 0;
			pwm_SwapBuffers();
		}
	}
}
