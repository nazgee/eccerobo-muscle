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
eMBErrorCode reg_Motor1( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode );
eMBErrorCode reg_Motor2( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode );

/* ----------------------- Static variables ---------------------------------*/


static reg_t r1 = {
		.id = 500,
		.value = 0,
		.handler = reg_Motor1
};

static reg_t r2 = {
		.id = 501,
		.value = 0,
		.handler = reg_Motor2
};
/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode reg_RW( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode ) {
	switch (eMode) {
		case MB_REG_READ: {
			*pucRegBuffer++ = ( UCHAR )( reg->value >> 8 );
			*pucRegBuffer++ = ( UCHAR )( reg->value & 0xFF );
		} break;
		case MB_REG_WRITE:
		default: {
			reg->value = *pucRegBuffer++ << 8;
			reg->value |= *pucRegBuffer++;
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

void MYMODBUS_Init(unsigned long baudrate)
{
	MOTOR_Init();
	REG_Register(&r1);
	REG_Register(&r2);

	const UCHAR ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
	eMBErrorCode eStatus;

	eStatus = eMBInit(MB_RTU, 0x0A, 0, baudrate, MB_PAR_NONE);

	eStatus = eMBSetSlaveID(0x34, TRUE, ucSlaveID, 3);

	/* Enable the Modbus Protocol Stack. */
	eStatus = eMBEnable();
}

void MYMODBUS_Manage(void) {
	(void) eMBPoll();
}

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    return MB_ENOREG;
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
