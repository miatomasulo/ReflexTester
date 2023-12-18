/*
 * Button_Driver.c
 *
 *  Created on: Sep 26, 2023
 *      Author: miat5
 */

/**
 * @file
 */
#include "Button_Driver.h"

/**
 * Configures the button to be used as an interrupt.
 * All values for macros used for pin, mode, speed, pull, and alternate
 * can be identified in the STM32F429 documentation
 * Enables proper clocks, initializes the GPIO, and enables the correct NVIC IRQ for the button
 */
void ButtonIntMode(){
	GPIO_InitTypeDef buttonIntConfig = {0};

	buttonIntConfig.Pin = GPIO_PIN_0;
	buttonIntConfig.Speed = GPIO_SPEED_FREQ_MEDIUM ;
	buttonIntConfig.Pull = OUTPUT_PP;
	buttonIntConfig.Alternate = GPIO_NOPULL;
	buttonIntConfig.Mode = GPIO_MODE_IT_RISING_FALLING;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	HAL_GPIO_Init(GPIOA, &buttonIntConfig);
	HAL_NVIC_EnableIRQ(EXTI0_IRQ_NUMBER);
}
