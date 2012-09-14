/*
 * mypwm.c
 *
 *  Created on: Sep 11, 2012
 *      Author: nazgee
 */


#include "pwm.h"
#include "../led.h"

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <string.h>

struct mypwm {
	duty_t period;
	uint8_t current_channel;

	volatile uint8_t dirty;
	uint8_t channels_number;
	uint8_t channels_number_dirty;

	pwm_desc_t channels_config[PWM_CHANNELS];
	pwm_desc_t channels_buffer[2][PWM_CHANNELS];
	pwm_desc_ptr channels;
	pwm_desc_ptr channels_dirty;

	duty_t deltas_buffer[2][PWM_CHANNELS + 1];
	duty_ptr deltas;
	duty_ptr deltas_dirty;
};

static struct mypwm this = {
	.channels_number = 0,
	.channels_number_dirty = 0,
	.current_channel = 0,
};

static inline void pwm_WaitForSync(void)
{
	while(this.dirty);
}

static inline void pwm_SwapBuffers(void)
{
	static int swap = 0;
	swap++;

	this.channels = this.channels_buffer[swap % 2];
	this.channels_dirty = this.channels_buffer[(swap + 1) % 2];

	this.deltas = this.deltas_buffer[swap % 2];
	this.deltas_dirty = this.deltas_buffer[(swap + 1) % 2];

	this.channels_number = this.channels_number_dirty;
}

static int pwm_CompareDuties(const void *cmp1, const void *cmp2) {
	const pwm_desc_ptr pwm1 = (pwm_desc_ptr)cmp1;
	const pwm_desc_ptr pwm2 = (pwm_desc_ptr)cmp2;
	return pwm1->duty - pwm2->duty;
}

// prescaler = 0;
#define ENABLE_CLOCK() do { TCNT1 = 0; TCCR1B |= _BV(CS11); } while(0)
#define DISABLE_CLOCK() do { TCCR1B &= ~(_BV(CS12)|_BV(CS11)|_BV(CS10)); TIFR |= _BV(OCIE1A); } while(0)

void PWM_Init(duty_t period)
{
	this.period = period;
	pwm_SwapBuffers();

	TIMSK |= _BV(OCIE1A);
	TCCR1B = _BV(WGM12);	// CTC mode is WGM=4

	OCR1A = 1000;
	ENABLE_CLOCK();
}

// XXX channels are double buffered! we need to use indexes here!
uint8_t PWM_Register(void* userdata, state_handler_t onStart, state_handler_t onCycle, duty_t duty)
{
	// XXX assert that there are some channels left for registration
	uint8_t id = this.channels_number_dirty;
	pwm_desc_ptr channel = &this.channels_config[id];
	channel->userdata = userdata;
	channel->onStart = onStart;
	channel->onCycle = onCycle;

	PWM_Duty(id, duty);

	this.channels_number_dirty++;

	return id;
}

inline void PWM_Duty(uint8_t id, duty_t duty)
{
	pwm_desc_ptr channel = &this.channels_config[id];
	channel->duty = duty;
}

static void inline pwm_SortChannels(void) {
	// copy all configs to a temporary storage
	memcpy(this.channels_dirty, this.channels_config, sizeof(this.channels_config[0]) * this.channels_number_dirty);

	// sort all the configs in a temporary storage
	qsort(this.channels_dirty, this.channels_number_dirty, sizeof(this.channels_dirty[0]), pwm_CompareDuties);

	// calculate deltas to be used in ISR
	duty_t prev_duty = 0;
	for (uint8_t i = 0; i < this.channels_number_dirty; i++) {
		this.deltas_dirty[i] = this.channels_dirty[i].duty - prev_duty;
		prev_duty = this.channels_dirty[i].duty;
	}
	// calculate last delta, that will restart the cycle
	this.deltas_dirty[this.channels_number_dirty] = this.period - prev_duty;
}

void PWM_Update(void)
{
	pwm_SortChannels();
	this.dirty = 1;
	pwm_WaitForSync();
}

void PWM_QuickUpdate(void)
{
	pwm_SortChannels();
	pwm_SwapBuffers();
}

ISR( TIMER1_COMPA_vect)
{
	DISABLE_CLOCK();

	if (this.current_channel < this.channels_number) {
		do {
			pwm_desc_ptr channel = &this.channels[this.current_channel];
			channel->onCycle(channel->userdata);
		} while ((this.deltas[++this.current_channel] == 0)
				&& (this.current_channel < this.channels_number));
	} else {
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

	OCR1A = this.deltas[this.current_channel];
	ENABLE_CLOCK();
}
