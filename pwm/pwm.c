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
#define ENABLE_CLOCK() do { CLEAR_TIMER_INTERRUPT(); TCNT1 = 0; TCCR1B |= _BV(CS12)|_BV(CS12); } while(0)
#define DISABLE_CLOCK() do { TCCR1B &= ~(_BV(CS12)|_BV(CS11)|_BV(CS10)); } while(0)

void PWM_Init(duty_t period)
{
	INIT_LIST_HEAD(&thiz.channels);
	thiz.period = period;

	TIMSK |= _BV(OCIE1A);	// enable OCIE1A interrupt
	TCCR1B = _BV(WGM12);	// set timer to CTC mode; WGM=4
}

void pwm_InsertSorted(pwm_desc_ptr new_channel) {
	struct list_head *i;
	struct pwm_desc *channel_i;

	list_for_each(i, &thiz.channels) {
		channel_i = list_entry(i, struct pwm_desc, node);
		if (new_channel->duty < channel_i->duty) {
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
			OCR1A = 3;	// XXX generate dummy interrupt soon
			ENABLE_CLOCK();
		}
	}
}

static inline void pwm_Sanitize(pwm_desc_ptr channel)
{
	if (channel->duty >= thiz.period) {
		channel->duty = thiz.period - 2;
	}
}

void PWM_Register(pwm_desc_ptr channel)
{
	pwm_Sanitize(channel);
	pwm_InsertSorted(channel);
}

void PWM_Duty(pwm_desc_ptr channel, duty_t duty)
{
	channel->duty = duty;
	pwm_Sanitize(channel);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		list_del(&channel->node);
		pwm_InsertSorted(channel);
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
			curr->onStart(curr->userdata);
		}

		pwm_desc_ptr first = list_first_entry(head, pwm_desc_t, node);
		OCR1A = first->duty;
	} else {
		curr = list_prepare_entry(curr, &thiz.channels, node);
		// call onCycle for all the channels with current timestamp
		list_for_each_entry_continue(curr, head, node) {
			curr->onCycle(curr->userdata);

			if (list_is_last(&curr->node, head)) {
				end_of_cycle = 1;
			} else {
				// fall back if next entry have bigger duty-
				// we'll have to wait for it till the next cycle
				pwm_desc_ptr next = list_entry(curr->node.next, pwm_desc_t, node);
				if (curr->duty < next->duty) {
					break;
				}
			}
		}

		if (!end_of_cycle) {
			pwm_desc_ptr next = list_entry(curr->node.next, pwm_desc_t, node);
			OCR1A = next->duty - curr->duty;
		} else {
			pwm_desc_ptr prev = list_entry(curr->node.prev, pwm_desc_t, node);
			OCR1A = thiz.period - prev->duty;
		}
	}

	ENABLE_CLOCK();
}
