/*
 * reg.c
 *
 *  Created on: Aug 16, 2012
 *      Author: nazgee
 */


#include "reg.h"
#include "defines.h"

static struct {
	reg_ptr_t registers[REGISTERS_NUMBER];
} reg;

/**
 * Returns reg_ptr_t* pointing to reg_t entry matching given id.
 * @param id
 * @return
 */
static reg_ptr_t* reg_Find(reg_id_t id) {

	reg_ptr_t* ret = NULL;
	for (int i=0; i<REGISTERS_NUMBER; i++) {
		if (reg.registers[i] != NULL) {
			if (reg.registers[i]->id == id) {
				ret = &reg.registers[i];
				break;
			}
		} else if (ret == NULL) {
			ret = &reg.registers[i];
		}
	}
	return ret;
}

void REG_Register(reg_ptr_t reg2register) {
	*reg_Find(reg2register->id) = reg2register;
}



eMBErrorCode REG_Handle( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode ) {
    eMBErrorCode    eStatus = MB_ENOERR;
//    return eStatus;

    for (int i=0; i<usNRegs; i++) {
    	reg_ptr_t* r = reg_Find(usAddress+i);
    	if ((r == NULL) || ((*r) == NULL))
    		return MB_ENOREG;
    }

    for (int i=0; i<usNRegs; i++) {
    	reg_ptr_t r = *reg_Find(usAddress+i);
    	eMBErrorCode status = r->handler(r, pucRegBuffer + 2*i, eMode);
    	if (status != MB_ENOERR) {
    		eStatus = status;
    	}
    }

    return eStatus;
}
