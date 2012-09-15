/*
 * FreeModbus Libary: ATMega168 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *   - Initial version and ATmega168 support
 * Modfications Copyright (C) 2006 Tran Minh Hoang:
 *   - ATmega8, ATmega16, ATmega32 support
 *   - RS485 support for DS75176
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
 * File: $Id: portserial.c,v 1.6 2006/09/17 16:45:53 wolti Exp $
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "port.h"
#include "led.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#define UART_PRESCALER(bitrate) (((F_CPU / (bitrate * 16UL))) - 1)

void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
#ifdef RTS_ENABLE
    UCSRB |= _BV( TXEN ) | _BV(TXCIE);
#else
    UCSRB |= _BV( TXEN );
#endif

    if( xRxEnable )
    {
        UCSRB |= _BV( RXEN ) | _BV( RXCIE );
    }
    else
    {
        UCSRB &= ~( _BV( RXEN ) | _BV( RXCIE ) );
    }

    if( xTxEnable )
    {
        UCSRB |= _BV( TXEN ) | _BV( UDRIE );
#ifdef RTS_ENABLE
        RTS_HIGH;
#endif
    }
    else
    {
        UCSRB &= ~( _BV( UDRIE ) );
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    UCHAR ucUCSRC = 0;

    /* prevent compiler warning. */
    (void)ucPORT;
	
	UBRRH = (UART_PRESCALER(ulBaudRate) >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
	UBRRL = UART_PRESCALER(ulBaudRate); // Load lower 8-bits of the baud rate value into the low byte of the UBRR register

    switch ( eParity )
    {
        case MB_PAR_EVEN:
            ucUCSRC |= _BV( UPM1 );
            break;
        case MB_PAR_ODD:
            ucUCSRC |= _BV( UPM1 ) | _BV( UPM0 );
            break;
        case MB_PAR_NONE:
            break;
    }

    switch ( ucDataBits )
    {
        case 8:
            ucUCSRC |= _BV( UCSZ0 ) | _BV( UCSZ1 );
            break;
        case 7:
            ucUCSRC |= _BV( UCSZ1 );
            break;
    }


    UCSRC = _BV( URSEL ) | ucUCSRC;

    vMBPortSerialEnable( FALSE, FALSE );
    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    UDR = ucByte;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = UDR;
    return TRUE;
}

//ISR( USART_UDRE_vect )
ISR( USARTUDRE_vect )
{
//	LED_Toggle(_BV(1));
    pxMBFrameCBTransmitterEmpty(  );
}

//ISR( USART_RXC_vect )
ISR( USARTRXC_vect )
{
//	LED_Toggle(_BV(2));
    pxMBFrameCBByteReceived(  );
}

#ifdef RTS_ENABLE
SIGNAL( SIG_UART_TRANS )
{
    RTS_LOW;
}
#endif

