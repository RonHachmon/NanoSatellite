/*
 * dht.h
 *
 *  Created on: Mar 19, 2025
 *      Author: 97254
 */

#ifndef INC_DHT_H_
#define INC_DHT_H_

#include "stm32l4xx_hal.h"

#include "main.h"

#define DHT_OK 1
#define DHT_FAIL 0

typedef struct DHT{
	TIM_HandleTypeDef* micro_clock;
	GPIO_TypeDef* m_GPIOx;
	uint16_t m_pin_id;
}DHT;

typedef struct CelsiusAndHumidity{
	uint8_t humidity_integral;
	uint8_t humidity_fractional;
	uint8_t tempature_integral;
	uint8_t tempature_fractional;

}CelsiusAndHumidity;

void init_DHT(DHT* dht, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,TIM_HandleTypeDef* clock);


void print_CelsiusAndHumidity(CelsiusAndHumidity* res);

uint8_t read_DHT(DHT* dht,CelsiusAndHumidity* res);



#endif /* INC_DHT_H_ */
