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
#include <util/atomic.h>

#include <string.h>

struct mypwm {
	duty_t period;
	struct list_head channels;
};

static struct mypwm thiz =
{
};

// prescaler = 0;
#define CLEAR_TIMER_INTERRUPT() TIFR |= _BV(OCF1A)
#define ENABLE_CLOCK() do { CLEAR_TIMER_INTERRUPT(); TCNT1 = 0; TCCR1B |= _BV(CS10)|_BV(CS10); } while(0)
#define DISABLE_CLOCK() do { TCCR1B &= ~(_BV(CS12)|_BV(CS11)|_BV(CS10)); } while(0)
#define CLOCKS_ISR_EXIT 10

void PWM_Init(duty_t period)
{
	INIT_LIST_HEAD(&thiz.channels);
	thiz.period = period;

	TIMSK |= _BV(OCIE1A);	// enable OCIE1A interrupt
	TCCR1B = _BV(WGM12);	// set timer to CTC mode; WGM=4
}

static inline void pwm_Sanitize(pwm_desc_ptr channel)
{
	if (channel->duty_target >= thiz.period) {
		channel->duty_target = thiz.period - 2;
	}
}

static inline duty_t pwm_GetTargetTick(pwm_desc_ptr channel)
{
	if (channel->phase) {
		return thiz.period - channel->duty_target;
	} else {
		return channel->duty_target;
	}
}

static inline duty_t pwm_GetCurrentTick(pwm_desc_ptr channel)
{
	if (channel->phase) {
		return thiz.period - channel->duty_current;
	} else {
		return channel->duty_current;
	}
}

void pwm_InsertSorted(pwm_desc_ptr new_channel) {
	struct list_head *i;
	struct pwm_desc *channel_i;

	list_for_each(i, &thiz.channels) {
		channel_i = list_entry(i, struct pwm_desc, node);
		if (pwm_GetCurrentTick(new_channel) < pwm_GetCurrentTick(channel_i)) {
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				// insert channel before i
				list_add_tail(&new_channel->node, i);
			}
			return;
		}
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// insert channel at the end of the list
		list_add_tail(&new_channel->node, &thiz.channels);
		if (list_is_singular(&thiz.channels)) {
			OCR1A = 4;	// XXX generate dummy interrupt soon
			ENABLE_CLOCK();
		}
	}
}

void PWM_Register(pwm_desc_ptr channel)
{
	pwm_Sanitize(channel);
	if (channel->phase) {
		state_handler_t tmp = channel->onDuty;
		channel->onDuty = channel->onPeriodFinished;
		channel->onPeriodFinished = tmp;
	}
	if (channel->duty_step == 0) {
		channel->duty_step = thiz.period;
	}
	pwm_InsertSorted(channel);
}

void PWM_Duty(pwm_desc_ptr channel, duty_t duty)
{
	channel->duty_target = duty;
	pwm_Sanitize(channel);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		list_del(&channel->node);
		pwm_InsertSorted(channel);
	}
}
/**
 * @return != 0 if reordering is needed after advancing ticks
 */
static inline uint8_t pwm_AdvanceCurrentTick(pwm_desc_ptr channel)
{
	if (channel->duty_current == channel->duty_target) {
		return 0;
	}

	if (channel->duty_current < channel->duty_target) {
		duty_t diff = channel->duty_target - channel->duty_current;
		duty_t step = diff < channel->duty_step ? diff : channel->duty_step;

		// advance
		channel->duty_current += step;
	} else {
		duty_t diff = channel->duty_current - channel->duty_target;
		duty_t step = diff < channel->duty_step ? -diff : -channel->duty_step;

		// advance
		channel->duty_current += step;

	}
	return 1;
}

static inline void pwm_UpdateOrdering(void) {
	pwm_desc_ptr cur;
	pwm_desc_ptr tmp;
	struct list_head* head = &thiz.channels;
	LIST_HEAD(pending_update);

	list_for_each_entry_safe(cur, tmp, head, node) {
		if (pwm_AdvanceCurrentTick(cur)) {
			list_move(&cur->node, &pending_update);
		}
	}

	list_for_each_entry_safe(cur, tmp, &pending_update, node) {
		list_del(&cur->node);
		pwm_InsertSorted(cur);
	}
}

ISR( TIMER1_COMPA_vect)
{
	static uint8_t end_of_cycle;
	static pwm_desc_ptr curr;

	if (list_empty(&thiz.channels)) {
		return;
	}

	DISABLE_CLOCK();

	struct list_head* head = &thiz.channels;

	if (end_of_cycle) {
		end_of_cycle = 0;
		list_for_each_entry(curr, head, node) {
			curr->onDuty(curr->userdata);
		}
		pwm_UpdateOrdering();

		curr = NULL;
		pwm_desc_ptr first = list_first_entry(head, pwm_desc_t, node);
		OCR1A = pwm_GetCurrentTick(first);
	} else {
		curr = list_prepare_entry(curr, &thiz.channels, node);
		// call onCycle for all the channels with current timestamp
		duty_t current_timestamp = 0;
		list_for_each_entry_continue(curr, head, node) {
			curr->onPeriodFinished(curr->userdata);
			current_timestamp = pwm_GetCurrentTick(curr);

			if (list_is_last(&curr->node, head)) {
				end_of_cycle = 1;
			} else {
				// bail out if next entry has bigger timestamp-
				// we'll have to wait for it till the next cycle
				pwm_desc_ptr next = list_entry(curr->node.next, pwm_desc_t, node);
				if (current_timestamp < pwm_GetCurrentTick(next)) {
					break;
				}
			}

		}

		if (!end_of_cycle) {
			pwm_desc_ptr next = list_entry(curr->node.next, pwm_desc_t, node);
			OCR1A = pwm_GetCurrentTick(next) - current_timestamp;
		} else {
			OCR1A = thiz.period - current_timestamp;
		}
	}

	OCR1A += 1;
	ENABLE_CLOCK();
}
