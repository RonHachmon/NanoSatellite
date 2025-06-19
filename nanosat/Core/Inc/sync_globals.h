/*
 * global_queues.h
 *
 *  Created on: Apr 2, 2025
 *      Author: 97254
 */

#ifndef INC_SYNC_GLOBALS_H_
#define INC_SYNC_GLOBALS_H_

#include "cmsis_os2.h"

#define FLAG_SET_TIME           0x00000001UL

#define FLAG_KEEP_ALIVE         0x00000002UL
#define FLAG_EVENT              0x00000004UL
#define FLAG_REPORTS            0x00000008UL

extern osMessageQueueId_t g_sensor_queue;

extern osMessageQueueId_t g_event_queue;

extern osMessageQueueId_t g_limits_change;

extern osMessageQueueId_t g_flah_limits_change;
extern osMessageQueueId_t g_sensor_limits_change;

extern osEventFlagsId_t g_evtID;

void global_queues_init(void);

#endif /* INC_SYNC_GLOBALS_H_ */
