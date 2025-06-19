/*
 * keep_alive_task.c
 *
 *  Created on: Apr 7, 2025
 *      Author: 97254
 */
#include <string.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ff_gen_drv.h"

#include "tasks/keep_alive_task.h"
#include "utils/send_queue.h"
#include "altair/message_handler.h"

#include "sensor_data.h"
#include "sync_globals.h"
#include "DateTime.h"



void keep_alive_task(void* context)
{
	Queue* transmit_queue = (Queue*) context;

	while(UNINTILIZED_MODE == g_latest_sensor_data.mode){
		osDelay(100);
	}



	while(1){
		send_keep_alive_packet(transmit_queue);

		osEventFlagsSet(g_evtID, FLAG_KEEP_ALIVE);

		osDelay(6000);
	}
}





