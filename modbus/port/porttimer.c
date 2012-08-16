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
#define MB_TIMER_PRESCALER      ( 8UL )
//#define MB_TIMER_TICKS          ( F_CPU / MB_TIMER_PRESCALER )
#define MB_50US_TICKS           ( 6UL )

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usTimerOCRADelta;

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
    vMBPortTimersDisable(  );

    /* Calculate overflow counter an OCR values for Timer1. */
    usTimerOCRADelta = usTim1Timerout50us * MB_50US_TICKS;

    // clear WGM bits
    TCCR1A &=  ~( _BV( WGM11 ) | _BV( WGM10 ));
    TCCR1B &=  ~( _BV( WGM13 ) | _BV( WGM12 ));
    // set CTC mode
    TCCR1B |=  ( _BV( WGM12 ));

    return TRUE;
}


inline void
vMBPortTimersEnable(  )
{
    TCNT1 = 0x0000;
    if( usTimerOCRADelta > 0 )
    {
    	// enable interrupt
        TIMSK |= _BV( OCIE1A );
        OCR1A = usTimerOCRADelta;
    }

    // set prescaler to enable clock
    TCCR1B |= _BV( CS11 );
}

inline void
vMBPortTimersDisable(  )
{
    /* Disable the timer by disabling clock */
    TCCR1B &= ~( _BV( CS12 ) | _BV( CS11 ) | _BV( CS10 ) );
    /* Disable interrupt */
    TIMSK &= ~( _BV( OCIE1A ) );
    /* Clear interrupt flag */
    TIFR |= _BV( OCF1A ) ;
}

ISR( TIMER1_COMPA_vect )
{
	LED_Toggle(LED_TIMER);

    ( void )pxMBPortCBTimerExpired(  );
}

