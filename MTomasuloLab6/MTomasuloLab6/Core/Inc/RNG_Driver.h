/*
 * RNG_Driver.h
 *
 *  Created on: Dec 1, 2023
 *      Author: miat5
 */

#ifndef INC_RNG_DRIVER_H_
#define INC_RNG_DRIVER_H_


#include "stm32f4xx_hal.h"

uint32_t getRandomNum();
void RNG_Init();

#endif /* INC_RNG_DRIVER_H_ */
