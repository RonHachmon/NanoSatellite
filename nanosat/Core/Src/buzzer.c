/*
 * buzzer.c
 *
 *  Created on: Mar 31, 2025
 *      Author: Ron Hachmon
 */

#include <stddef.h>

#include "buzzer.h"

static Buzzer buzzer ={0};

void init_Buzzer(TIM_HandleTypeDef* PMW_clock,uint32_t channel)
{
	buzzer.PMW_clock = PMW_clock;
	buzzer.channel = channel;
}


void buzzer_run(uint16_t time_to_run)
{
	if(NULL == buzzer.PMW_clock ){
		return;
	}

	HAL_TIM_Base_Start(buzzer.PMW_clock);
	HAL_TIM_PWM_Start(buzzer.PMW_clock, buzzer.channel);
	osDelay(time_to_run);

	HAL_TIM_PWM_Stop(buzzer.PMW_clock, buzzer.channel);
}


void buzzer_start()
{
	if(NULL == buzzer.PMW_clock ){
		return;
	}

	HAL_TIM_Base_Start(buzzer.PMW_clock);
	HAL_TIM_PWM_Start(buzzer.PMW_clock, buzzer.channel);
}


void buzzer_end()
{
	if(NULL == buzzer.PMW_clock ){
		return;
	}

	HAL_TIM_Base_Start(buzzer.PMW_clock);
	HAL_TIM_PWM_Stop(buzzer.PMW_clock, buzzer.channel);
}
