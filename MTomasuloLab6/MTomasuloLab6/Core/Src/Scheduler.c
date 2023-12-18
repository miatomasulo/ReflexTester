/*
 * Scheduler.c
 *
 *  Created on: Sep 5, 2023
 *      Author: miat5
 */

/*
 * @file
 */
#include "Scheduler.h"

static uint32_t scheduledEvents;

/**Schedules an event to be completed later
 *
 */
void addSchedulerEvent(uint32_t event){
	scheduledEvents |= event;
};

/**Removes a scheduled event (typically after it has been called and completed)
 *
 */
void removeSchedulerEvent(uint32_t event){
	scheduledEvents &= ~event;
};

/**Get function to check which events have been scheduled
 * @retval unsigned 32-bit value
 */
uint32_t getScheduledEvents(){
	return scheduledEvents;
};

