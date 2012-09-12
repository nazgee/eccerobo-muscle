/*
 * FreeModbus Libary: ATMega168 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.4 2006/09/03 11:53:10 wolti Exp $
 */

/* ----------------------- AVR includes -------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "led.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define TICKS_PER_50US           ( 6 ) // how many ticks 50us lasts

/* ----------------------- Static variables ---------------------------------*/
static USHORT   number_of_50us_periods;

/* ----------------------- Start implementation -----------------------------*/
BOOL
 xMBPortTimersInit(USHORT is50usTimeouts) {
	vMBPortTimersDisable();

	// set CTC mode
	TCCR2 |= (_BV( WGM21 ));
	OCR2 = TICKS_PER_50US * is50usTimeouts;

	return TRUE;
}


inline void
vMBPortTimersEnable(  )
 {
	TCNT2 = 0x00;

	// enable interrupt
	TIMSK |= _BV( OCIE2 );

	// set prescaler to enable clock
	TCCR2 |= _BV( CS22 ) ; // prescaler = 64
}

inline void
vMBPortTimersDisable(  )
{
    /* Stop the timer by disabling clock */
    TCCR2 &= ~( _BV( CS22 ) | _BV( CS21 ) | _BV( CS20 ) );
    /* Disable interrupt */
    TIMSK &= ~( _BV( OCIE2 ) );
    /* Clear interrupt flag */
    TIFR |= _BV( OCF2 );
}

ISR( TIMER2_COMP_vect )
{
//	LED_Toggle(LED_TIMER);
	( void )pxMBPortCBTimerExpired();

}

