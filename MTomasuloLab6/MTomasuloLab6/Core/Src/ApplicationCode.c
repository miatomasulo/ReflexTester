/*
 * ApplicationCode.c
 *
 *  Created on: Nov 30, 2023
 *      Author: miat5
 */
/**
 * @file
 */
#include "ApplicationCode.h"

/**
 * ApplicationInit() calls different initialization functions that enable the
 * Button interrupt, Random Number Generator, and LCD display screen.
 * It also adds events to the scheduler so that they may be called elsewhere
 * in the code when appropriate.
 */
void ApplicationInit(void)
{
	LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);

    RNG_Init();
    ButtonIntMode();

    addSchedulerEvent(LEVEL1);
    addSchedulerEvent(LEVEL2);
    addSchedulerEvent(LEVEL3);
}

/**The IRQ Handler: used when an interrupt is detected (ie button is pressed).
 *
 * When the button is pressed and the code enters the handler, buttonStatus() is called.
 * The following condition is then checked to make sure incrementCounter() and setValidInput()
 * are called at the appropriate time.
 */
void EXTI0_IRQHandler(){
	IRQ_disable(EXTI0_IRQ_NUMBER);

	buttonStatus();

	if(getLevelThree() && getValidInput()){

		incrementCounter();
		setValidInput();

	}

	clearInterruptPendingBit(14);
	IRQ_enable(EXTI0_IRQ_NUMBER);
}

/**Calls QuickDemo() to ensure the LCD is properly configured and working.
 *
 */
void RunDemoForLCD(void)
{
	QuickDemo();
}

