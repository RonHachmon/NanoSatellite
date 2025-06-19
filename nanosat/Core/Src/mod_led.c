/*
 * mod_led.c
 *
 *  Created on: Mar 6, 2025
 *      Author: Ron_Hachmon
 */


#include "mod_led.h"

//struct Led {
//	GPIO_TypeDef* m_GPIOx;
//	uint16_t m_pin_id;
//};


void led_create(Led* led, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	led->m_GPIOx = GPIOx;
	led->m_pin_id = GPIO_Pin;
}

void led_on(Led* led)
{
	HAL_GPIO_WritePin(led->m_GPIOx, led->m_pin_id, GPIO_PIN_SET);
}

void led_off(Led* led)
{
	HAL_GPIO_WritePin(led->m_GPIOx, led->m_pin_id, GPIO_PIN_RESET);
}

void led_toggle(Led* led)
{
	HAL_GPIO_TogglePin(led->m_GPIOx, led->m_pin_id);
}
