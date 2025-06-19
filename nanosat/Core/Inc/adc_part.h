/*
 * adc_part.h
 *
 *  Created on: Apr 2, 2025
 *      Author: 97254
 */

#ifndef INC_ADC_PART_H_
#define INC_ADC_PART_H_

#include "adc.h"

typedef struct {
    ADC_HandleTypeDef* analogic_clock;
} ADC_Part;

void ADC_init(ADC_Part* adc_part, ADC_HandleTypeDef* analogic_clock);

HAL_StatusTypeDef ADC_read_data(ADC_Part* adc_part, uint16_t* data);

#endif /* INC_ADC_PART_H_ */
