 /*
 * InterruptControl.h
 *
 *  Created on: Oct 6, 2023
 *      Author: miat5
 */

#ifndef INTERRUPTCONTROL_H_
#define INTERRUPTCONTROL_H_
#include "stm32f4xx_hal.h"


#define EXTI0_IRQ_NUMBER 6//*denotes proper interrupt name

#define NVIC_ISER0 ((volatile uint32_t*) 0xE000E100) //set enable
#define NVIC_ICER0 ((volatile uint32_t*) 0xE000E180) //clear enable
#define NVIC_ISPR0 ((volatile uint32_t*) 0xE000E200) //set pending
#define NVIC_ICPR0 ((volatile uint32_t*) 0xE000E280) //clear pending

typedef struct{
	//EXTI register map
	volatile uint32_t EXTI_IMR;
	volatile uint32_t EXTI_EMR;
	volatile uint32_t EXTI_RTSR;
	volatile uint32_t EXTI_FTSR;
	volatile uint32_t EXTI_SWIER;
	volatile uint32_t EXTI_PR;
}EXTI_RegDef_t;

#define APB2_BASE_ADDR 0x40010000
#define EXTI_BASE_ADDR (APB2_BASE_ADDR+0x3C00)
#define EXTI_ ((EXTI_RegDef_t*) EXTI_BASE_ADDR)

void IRQ_enable(uint8_t IRQ);
void IRQ_disable(uint8_t IRQ);
void IRQ_clearPending(uint8_t IRQ);
void IRQ_setPending(uint8_t IRQ);

void clearInterruptPendingBit(uint8_t pin); //in EXTI register


#endif /* INTERRUPTCONTROL_H_ */
