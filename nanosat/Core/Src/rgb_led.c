/*
 * rgb_led.c
 *
 *  Created on: Mar 10, 2025
 *      Author: 97254
 */

#include "rgb_led.h"

static RGB_Led rgb_Led;

static void RGB_Set(GPIO_PinState red_state,GPIO_PinState green_state, GPIO_PinState blue_state)
{
	HAL_GPIO_WritePin(rgb_Led.red_port, rgb_Led.red_pin, red_state);
	HAL_GPIO_WritePin(rgb_Led.green_port, rgb_Led.green_pin, green_state);
	HAL_GPIO_WritePin(rgb_Led.blue_port, rgb_Led.blue_pin, blue_state);

}

void RGB_LED_init( GPIO_TypeDef* red_GPIOx, uint16_t red_GPIO_Pin,GPIO_TypeDef* green_GPIOx, uint16_t green_GPIO_Pin,GPIO_TypeDef* blue_GPIOx, uint16_t blue_GPIO_Pin)
{
	rgb_Led.red_port = red_GPIOx;
	rgb_Led.red_pin = red_GPIO_Pin;

	rgb_Led.green_port = green_GPIOx;
	rgb_Led.green_pin = green_GPIO_Pin;

	rgb_Led.blue_port = blue_GPIOx;
	rgb_Led.blue_pin = blue_GPIO_Pin;
	rgb_Led.current_color = 0u;

	return;
}



void RGB_LED_Set_Color(RGB_Color_t color)
{

	rgb_Led.current_color = (uint8_t) color;
	RGB_Set(rgb_Led.current_color & 0x1, rgb_Led.current_color & 0x2, rgb_Led.current_color & 0x4);
}

void RGB_LED_next_Color()
{

	rgb_Led.current_color = (rgb_Led.current_color + 1) % 8;

	if (rgb_Led.current_color == 0) {
		rgb_Led.current_color += 1;
	}

	RGB_Set(rgb_Led.current_color & 0x1, rgb_Led.current_color & 0x2, rgb_Led.current_color & 0x4);
}

void RGB_LED_off()
{
	RGB_Set(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
}
