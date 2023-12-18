/*
 * InterruptControl.c
 *
 *  Created on: Oct 6, 2023
 *      Author: miat5
 */

/**
 * @file
 */
#ifndef INTERRUPTCONTROL_C_
#define INTERRUPTCONTROL_C_
#include "InterruptControl.h"

/*
 * The following functions are all used for dealing with an interrupt
 * Allows user to enable, disable, set, and clear interrupt bits
 * @param IRQ is the value of the interrupt number
 * @param pin gives the pin value of the external interrupt
 */

void IRQ_enable(uint8_t IRQ){
	if (IRQ < 32){
		*NVIC_ISER0  |= (1 << IRQ); //Interrupt Set Enable Register
	}
	else if (IRQ >= 32){
		int bitmask = (IRQ)%32;
		*NVIC_ISER0 |= (1 << bitmask);
	}
}

void IRQ_disable(uint8_t IRQ){
	if (IRQ < 32){
			*NVIC_ICER0  |= (1 << IRQ);//Interrupt Clear Enable Register
		}
		else if (IRQ >= 32){
			int bitmask = (IRQ)%32;
			*NVIC_ICER0 |= (1 << bitmask);
		}
}

void IRQ_setPending(uint8_t IRQ){
	if (IRQ < 32){
			*NVIC_ISPR0  |= (1 << IRQ);//Interrupt Set Pending Register
		}
		else if (IRQ >= 32){
			int bitmask = (IRQ)%32;
			*NVIC_ISPR0 |= (1 << bitmask);
		}
}

void IRQ_clearPending(uint8_t IRQ){
	if (IRQ < 32){
			*NVIC_ICPR0  |= (1 << IRQ);//Interrupt Clear Pending Register
		}
		else if (IRQ >= 32){
			int bitmask = (IRQ)%32;
			*NVIC_ICPR0 |= (1 << bitmask);
		}
}

void clearInterruptPendingBit(uint8_t pin){
	EXTI_->EXTI_PR |= (1 << pin);
}

#endif /* INTERRUPTCONTROL_C_ */
