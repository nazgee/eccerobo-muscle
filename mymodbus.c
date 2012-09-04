/*
 * mymodbus.c
 *
 *  Created on: Aug 13, 2012
 *      Author: nazgee
 */

/* ----------------------- AVR includes -------------------------------------*/
#include "avr/io.h"
#include "avr/interrupt.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

#include "regs/reg.h"
#include "motor/motor.h"

/* ----------------------- Defines ------------------------------------------*/
#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];

eMBErrorCode reg_Motor1( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode );
eMBErrorCode reg_Motor2( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode );

static reg_t r1 = {
		.id = 500,
		.value = 69,
		.handler = reg_Motor1
};

static reg_t r2 = {
		.id = 501,
		.value = 666,
		.handler = reg_Motor2
};
/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode reg_RW( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode ) {
	switch (eMode) {
		case MB_REG_READ: {
			*pucRegBuffer++ = ( unsigned char )( reg->value >> 8 );
			*pucRegBuffer++ = ( unsigned char )( reg->value & 0xFF );
		} break;
		case MB_REG_WRITE:
		default: {
			reg->value = ((reg_val_t)pucRegBuffer[0] << 8) | pucRegBuffer[1];
		} break;
	}
	return MB_ENOERR;
}

eMBErrorCode reg_Motor( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode, enum eMotor eMotor ) {
	reg_RW(reg, pucRegBuffer, eMode);

	switch (eMode) {
		case MB_REG_WRITE: {
			MOTOR_Set(eMotor, reg->value);
		} break;
		default:
			break;
	}
	return MB_ENOERR;
}

eMBErrorCode reg_Motor1( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode ) {
	return reg_Motor(reg, pucRegBuffer, eMode, MOTOR1);
}

eMBErrorCode reg_Motor2( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode ) {
	return reg_Motor(reg, pucRegBuffer, eMode, MOTOR2);
}

void MYMODBUS_Init(int baudrate)
{
	MOTOR_Init();
	REG_Register(&r1);
	REG_Register(&r2);

	const UCHAR ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
	eMBErrorCode eStatus;

	eStatus = eMBInit(MB_RTU, 0x0A, 0, baudrate, MB_PAR_NONE);

	eStatus = eMBSetSlaveID(0x34, TRUE, ucSlaveID, 3);
	sei( );

	/* Enable the Modbus Protocol Stack. */
	eStatus = eMBEnable();
}

void MYMODBUS_Manage(void) {
	(void) eMBPoll();
	usRegInputBuf[0]++;
	usRegInputBuf[1] = 666;
	usRegInputBuf[2] = 69;
}

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{
	return REG_Handle(pucRegBuffer, usAddress, usNRegs, eMode);
}


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
