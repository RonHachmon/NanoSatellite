/*
 * flash_task.h
 *
 *  Created on: Apr 6, 2025
 *      Author: 97254
 */

#ifndef INC_TASKS_FLASH_TASK_H_
#define INC_TASKS_FLASH_TASK_H_



#include "tasks/collector_task.h"


typedef enum {
    UPDATE_HUMIDITY_LIMIT = 0,
	UPDATE_VOLTAGE_LIMIT,
	UPDATE_LIGHT_LIMIT,
	UPDATE_MIN_TEMP_LIMIT,
	UPDATE_MAX_TEMP_LIMIT
} updateAttribute;

typedef struct UpdateSetting{
	updateAttribute update_attribute;
	uint8_t buffer[8];
}UpdateSetting;



void reset_flash();
void flash_task(void* context);
void read_settings(CollectorSetting* collector_settings);

#endif /* INC_TASKS_FLASH_TASK_H_ */
