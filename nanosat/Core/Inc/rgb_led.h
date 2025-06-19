/*
 * rgb_led.h
 *
 *  Created on: Mar 10, 2025
 *      Author: Ron Hachmon
 */

#ifndef INC_RGB_LED_H_
#define INC_RGB_LED_H_

#include "stm32l4xx_hal.h"


typedef enum {
    COLOR_OFF = 0,
    COLOR_RED,
    COLOR_GREEN,
	COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
	TOTAL_COLORS
} RGB_Color_t;



typedef struct RGB_Led{

	GPIO_TypeDef* red_port;
	uint16_t red_pin;

	GPIO_TypeDef* green_port;
	uint16_t green_pin;

	GPIO_TypeDef* blue_port;
	uint16_t blue_pin;

	uint8_t current_color;
} RGB_Led;




void RGB_LED_init(GPIO_TypeDef* red_GPIOx, uint16_t red_GPIO_Pin,GPIO_TypeDef* green_GPIOx, uint16_t green_GPIO_Pin,GPIO_TypeDef* blue_GPIOx, uint16_t blue_GPIO_Pin);


void RGB_LED_next_Color();

void RGB_LED_Set_Color(RGB_Color_t color);

void RGB_LED_off();

#endif /* INC_RGB_LED_H_ */

