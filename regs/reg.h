/*
 * reg.h
 *
 *  Created on: Aug 16, 2012
 *      Author: nazgee
 */

#ifndef REG_H_
#define REG_H_

#include <stdint.h>
#include <mb.h>

#define REGISTERS_NUMBER 2

typedef struct reg reg_t;
typedef reg_t* reg_ptr_t;

typedef         eMBErrorCode( *regHandler ) ( reg_ptr_t reg, UCHAR * pucRegBuffer, eMBRegisterMode eMode );
typedef uint16_t reg_id_t;
typedef uint16_t reg_val_t;
struct reg {
	reg_id_t id;
	reg_val_t value;
	regHandler handler;
};

void REG_Register(reg_ptr_t reg2register);
eMBErrorCode REG_Handle( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode );


#endif /* REG_H_ */
