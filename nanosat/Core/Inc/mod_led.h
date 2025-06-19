/*
 * mod_led.h
 *
 *  Created on: Mar 6, 2025
 *      Author: Ron_Hachmon
 */

#ifndef INC_MOD_LED_H_
#define INC_MOD_LED_H_

#include "stm32l4xx_hal.h"



typedef struct Led {
	GPIO_TypeDef* m_GPIOx;
	uint16_t m_pin_id;
} Led;

//typedef struct Led Led;


void led_create(Led* led, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void led_off(Led* led);

void led_on(Led* led);

void led_toggle(Led* led);


#endif /* INC_MOD_LED_H_ */
