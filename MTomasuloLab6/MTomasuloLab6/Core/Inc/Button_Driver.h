/*
 * Button_Driver.h
 *
 *  Created on: Sep 26, 2023
 *      Author: miat5
 */

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_
#include "InterruptControl.h"


//User Button Port A Pin 0 (PA0)
#define BUTTON_PORT_VALUE GPIOA
#define BUTTON_PIN_VALUE 0
#define BUTTON_PRESSED 1
#define UNPRESSED 0

void ButtonInit();
void ClockEnable();
int PressedNotPressed(); //return a boolean --> true if pressed / false if not
void ButtonIntMode();

#endif /* BUTTON_DRIVER_H_ */
