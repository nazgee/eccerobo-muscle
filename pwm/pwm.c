/*
 * pwm.c
 *
 *  Created on: Sep 11, 2012
 *      Author: nazgee
 */

#include "pwm.h"

#define F_PWM         10L               // PWM-Frequencu in Hz
#define PWM_PRESCALER 8                  // Timer prescaler
#define PWM_STEPS     256                // PWM resolution
#define PWM_PORT      PORTB              // Port for PWM
#define PWM_DDR       DDRB               // DDR for PWM
#define PWM_CHANNELS  8                  // PWM channels number
// ab hier nichts ändern, wird alles berechnet

#define T_PWM (F_CPU/(PWM_PRESCALER*F_PWM*PWM_STEPS)) // Systemtakte pro PWM-Takt
#if ((T_PWM*PWM_PRESCALER)<(111+5))
#error T_PWM error too small, F_CPU  needs to be increased or F_PWM/PWM_STEPS have to be reduced
#endif

#if ((T_PWM*PWM_STEPS)>65535)
#error period of the PWM too big!  F_PWM or PWM_PRESCALER increase.
#endif
// includes

#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t pwm_timing[PWM_CHANNELS + 1];
uint16_t pwm_timing_tmp[PWM_CHANNELS + 1];

uint8_t pwm_mask[PWM_CHANNELS + 1];
uint8_t pwm_mask_tmp[PWM_CHANNELS + 1];

uint8_t pwm_setting[PWM_CHANNELS];
uint8_t pwm_setting_tmp[PWM_CHANNELS + 1];

volatile uint8_t pwm_cnt_max = 1; // init to 1 is important
volatile uint8_t pwm_sync; // update is now available

uint16_t *isr_ptr_time = pwm_timing;
uint16_t *main_ptr_time = pwm_timing_tmp;

uint8_t *isr_ptr_mask = pwm_mask;
uint8_t *main_ptr_mask = pwm_mask_tmp;

//

/**
 * Replaces the pointers between main and ISR program
 * This must be done in a subroutine,
 * to avoid an intermediate storage by the compiler
 */
void replace_pointers(void)
{
	uint16_t *tmp_ptr16;
	uint8_t *tmp_ptr8;

	tmp_ptr16 = isr_ptr_time;
	isr_ptr_time = main_ptr_time;
	main_ptr_time = tmp_ptr16;

	tmp_ptr8 = isr_ptr_mask;
	isr_ptr_mask = main_ptr_mask;
	main_ptr_mask = tmp_ptr8;
}

/**
 * Update PWM calculated from the PWM settings
 * The new values will be used in ISR
 */
void pwm_update(void)
{

	uint8_t i, j, idx_of_min, channels;
	uint8_t mask_tmp, mask_new, tmp_mask;
	uint8_t min, tmp_set;

	// Mask for PWM start calculating
	// Generate the same bitmasks and copy PWM values

	mask_tmp = 1;
	mask_new = 0;
	for (i = 1; i <= (PWM_CHANNELS); i++) {
		main_ptr_mask[i] = ~mask_tmp;	// XXX do we have to clear one-by-one? would not it be easier to zero it in the first place?
		pwm_setting_tmp[i] = pwm_setting[i - 1];
		if (pwm_setting_tmp[i] != 0)
			mask_new |= mask_tmp;
		mask_tmp <<= 1;
	}
	main_ptr_mask[0] = mask_new;

	// PWM settings sorting

	for (i = 1; i <= PWM_CHANNELS; i++) {
		min = PWM_STEPS - 1;
		idx_of_min = i;
		for (j = i; j <= PWM_CHANNELS; j++) {
			if (pwm_setting_tmp[j] < min) {
				min = pwm_setting_tmp[j];
				idx_of_min = j;
			}
		}
		if (idx_of_min != i) {
			// exchange pwm_setting
			tmp_set = pwm_setting_tmp[idx_of_min];
			pwm_setting_tmp[idx_of_min] = pwm_setting_tmp[i];
			pwm_setting_tmp[i] = tmp_set;

			// exchange masks
			tmp_mask = main_ptr_mask[idx_of_min];
			main_ptr_mask[idx_of_min] = main_ptr_mask[i];
			main_ptr_mask[i] = tmp_mask;
		}
	}

	// Combina channels with same settings, get rid of 0 (if present)

	channels = PWM_CHANNELS;
	i = 1;

	while (channels > i) {
		while (((pwm_setting_tmp[i] == pwm_setting_tmp[i + 1])
				|| (pwm_setting_tmp[i] == 0)) && (channels > i)) {

			// Consecutive values are combined
			if (pwm_setting_tmp[i] != 0)
				main_ptr_mask[i + 1] &= main_ptr_mask[i];

			// Move all entries one position down
			for (j = i; j < channels; j++) {
				pwm_setting_tmp[j] = pwm_setting_tmp[j + 1];
				main_ptr_mask[j] = main_ptr_mask[j + 1];
			}
			channels--;
		}
		i++;
	}

    // We deal with last channel in a different way
    // Do not compare with the possible successor to delete, only
    // Applies only in the special case when all channels are 0
	if (pwm_setting_tmp[i] == 0)
		channels--;

	// Calculate time values

	if (channels == 0) { // Special case, when we have only 0 values
		main_ptr_time[0] = (uint16_t) T_PWM * PWM_STEPS / 2;
		main_ptr_time[1] = (uint16_t) T_PWM * PWM_STEPS / 2;
		channels = 1;
	} else {
		i = channels;
		main_ptr_time[i] = (uint16_t) T_PWM * (PWM_STEPS - pwm_setting_tmp[i]);
		tmp_set = pwm_setting_tmp[i];
		i--;
		for (; i > 0; i--) {
			main_ptr_time[i] = (uint16_t) T_PWM
					* (tmp_set - pwm_setting_tmp[i]);
			tmp_set = pwm_setting_tmp[i];
		}
		main_ptr_time[0] = (uint16_t) T_PWM * tmp_set;
	}

	pwm_sync = 0; // set to 1 from interrupt
	while (pwm_sync == 0)
		;

	// XXX this is all fucked! it is not atomic! it should be done other way around to prevent races
	// which still can be get
	// -- HERE --

	// pointers swap
	cli();
	replace_pointers();
	pwm_cnt_max = channels;
	sei();
}

// Timer 1 Output COMPARE A Interrupt

ISR( TIMER1_COMPB_vect)
{
	static uint8_t pwm_cnt;
	uint8_t tmp;

	OCR1A += isr_ptr_time[pwm_cnt];
	tmp = isr_ptr_mask[pwm_cnt];

	if (pwm_cnt == 0) {
		PWM_PORT = tmp; // beginning PWM cycle
		pwm_cnt++;
	} else {
		PWM_PORT &= tmp; // clearing PWM ports
		if (pwm_cnt == pwm_cnt_max) {
			pwm_sync = 1;
			pwm_cnt = 0;
		} else
			pwm_cnt++;
	}
}
//
//int main(void)
//{
//
//	// PWM Port einstellen
//
//	PWM_DDR = 0xFF; // Port als Ausgang
//	// zusätzliche PWM-Ports hier setzen
//
//	// Timer 1 OCRA1, als variablen Timer nutzen
//
//	TCCR1B = 2; // Timer läuft mit Prescaler 8
//	TIMSK |= (1 << OCIE1A); // Interrupt freischalten
//
//	sei(); // Interrupts global einschalten
//
//	/******************************************************************/
//// nur zum testen, in der Anwendung entfernen
//	/*
//	 // Test values
//	 volatile uint8_t tmp;
//	 const uint8_t t1[8]={255, 40, 3, 17, 150, 99, 5, 9};
//	 const uint8_t t2[8]={27, 40, 3, 0, 150, 99, 5, 9};
//	 const uint8_t t3[8]={27, 40, 3, 17, 3, 99, 3, 0};
//	 const uint8_t t4[8]={0, 0, 0, 0, 0, 0, 0, 0};
//	 const uint8_t t5[8]={9, 1, 1, 1, 1, 1, 1, 1};
//	 const uint8_t t6[8]={33, 33, 33, 33, 33, 33, 33, 33};
//	 const uint8_t t7[8]={0, 0, 0, 0, 0, 0, 0, 88};
//
//
//	 // Messung der Interruptdauer
//	 tmp =1;
//	 tmp =2;
//	 tmp =3;
//
//	 // Debug
//
//	 memcpy(pwm_setting, t1, 8);
//	 pwm_update();
//
//	 memcpy(pwm_setting, t2, 8);
//	 pwm_update();
//
//	 memcpy(pwm_setting, t3, 8);
//	 pwm_update();
//
//	 memcpy(pwm_setting, t4, 8);
//	 pwm_update();
//
//	 memcpy(pwm_setting, t5, 8);
//	 pwm_update();
//
//	 memcpy(pwm_setting, t6, 8);
//	 pwm_update();
//
//	 memcpy(pwm_setting, t7, 8);
//	 pwm_update();
//	 */
//	/******************************************************************/
//
//	while (1)
//		;
//	return 0;
//}
