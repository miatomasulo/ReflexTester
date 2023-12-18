/*
 * Scheduler.h
 *
 *  Created on: Sep 5, 2023
 *      Author: miat5
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "Button_Driver.h"

#define POLL_BUTTON 1<<0
#define LEVEL1 1<<1
#define LEVEL2 1<<2
#define LEVEL3 1<<3

uint32_t getScheduledEvents();
void addSchedulerEvent(uint32_t);
void removeSchedulerEvent(uint32_t);


#endif /* SCHEDULER_H_ */
