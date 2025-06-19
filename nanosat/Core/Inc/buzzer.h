/*
 * buzzer.h
 *
 *  Created on: Mar 31, 2025
 *      Author: 97254
 */

#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "stm32l4xx_hal.h"
typedef struct Buzzer {

	TIM_HandleTypeDef* PMW_clock;
	uint32_t channel;

} Buzzer;


void init_Buzzer(TIM_HandleTypeDef* PMW_clock,uint32_t channel);


void buzzer_run(uint16_t time_to_run);
void buzzer_start();
void buzzer_end();

#endif /* INC_BUZZER_H_ */
