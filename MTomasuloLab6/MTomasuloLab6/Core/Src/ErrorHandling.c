/*
 * ErrorHandling.c
 *
 *  Created on: Nov 4, 2023
 *      Author: miat5
 */

/**
 * @file
 */
#include "ErrorHandling.h"


/**
 * When an error occurs, code is sent into an infinite while loop.
 * Helpful for debugging purposes.
 * @param trueOrFalse takes in a boolean value of TRUE or FALSE
 */
void APPLICATION_ASSERT(bool trueOrFalse)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (trueOrFalse == 0)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

