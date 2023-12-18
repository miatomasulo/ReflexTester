/*
 * RNG_Driver.c
 *
 *  Created on: Dec 1, 2023
 *      Author: miat5
 */

/**
 * @file
 */
#include "RNG_Driver.h"
RNG_HandleTypeDef RNGInitConfig = {0};

/**@brief
 * Initializes the Random Number Generator peripheral
 * Configures the RNG and enables the clock
*/
void RNG_Init(){

	RNGInitConfig.Instance = RNG;    /*!< Register base address   */

	__HAL_RCC_RNG_CLK_ENABLE();
	__HAL_RNG_ENABLE(&RNGInitConfig);
	HAL_RNG_Init(&RNGInitConfig);

}

 /**Function returns a randomly generated number.
 *Used for randomization of other game properties: shapes and locations
 *@retval unsigned 32-bit integer to be used as the random number
 */
uint32_t getRandomNum(){

	uint32_t randomNumber;

	HAL_RNG_GenerateRandomNumber(&RNGInitConfig, &randomNumber);

	return randomNumber;
}

