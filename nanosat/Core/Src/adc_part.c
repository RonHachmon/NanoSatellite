/*
 * adc_part.c
 *
 *  Created on: Mar 31, 2025
 *      Author: 97254
 */

#include <stddef.h>

#include "adc_part.h"


void ADC_init(ADC_Part* adc_part, ADC_HandleTypeDef* analogic_clock)
{
	if(NULL == adc_part){
		return;
	}
	adc_part->analogic_clock = analogic_clock;
}

HAL_StatusTypeDef ADC_read_data(ADC_Part* adc_part, uint16_t* data)
{
	if(NULL == adc_part ||  NULL == data ){
		return HAL_ERROR;
	}

	HAL_StatusTypeDef status;

	if( status = HAL_ADC_Start(adc_part->analogic_clock) != HAL_OK ){
		return status;
	}

	if( status = HAL_ADC_PollForConversion(adc_part->analogic_clock, 300) != HAL_OK ){
		return status;
	}

	HAL_ADC_PollForConversion(adc_part->analogic_clock, 300);
	*data = HAL_ADC_GetValue(adc_part->analogic_clock);

	return HAL_OK;
}
