/*
 * dht.c
 *
 *  Created on: Mar 19, 2025
 *      Author: 97254
 */
#include "dht.h"
#include "main.h"

#define TIMEOUT_THRESHOLD 100

static void set_as_input(DHT* dht);
static void set_as_output(DHT* dht);

static uint8_t read_bit(DHT* dht);

static uint8_t calc_bits(DHT* dht, CelsiusAndHumidity* res);
static uint8_t run(DHT* dht, CelsiusAndHumidity* res);

void init_DHT(DHT* dht, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin,TIM_HandleTypeDef* clock)
{
	if(dht == NULL || GPIOx == NULL || clock == NULL)
	{
		return;
	}

	dht->m_GPIOx=GPIOx;
	dht->m_pin_id = GPIO_Pin;
	dht->micro_clock = clock;
}


uint8_t read_DHT(DHT* dht, CelsiusAndHumidity* res)
{
	if(dht == NULL){
		return DHT_FAIL;
	}

//init phase
	HAL_TIM_Base_Start(dht->micro_clock);

	set_as_output(dht);

	HAL_GPIO_WritePin(dht->m_GPIOx, dht->m_pin_id, GPIO_PIN_RESET);

	osDelay(18);



////#ifdef osCMSIS
//	HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
//	osPriority_t previous_priority;
//	osThreadId_t thread_id;
//	if(osKernelGetState() == osKernelRunning)
//	{
//		thread_id = osThreadGetId();
//		previous_priority = osThreadGetPriority(thread_id);
//
//
//		osThreadSetPriority(thread_id, osPriorityHigh7);
//	}
//	// FreeRTOS Delay
////    HAL_Delay(18); // Bare Metal Delay
////#endif

    uint8_t status = run(dht, res);

////#ifdef CMSIS_OS2_H_
//  if(osKernelGetState() == osKernelRunning)
//    {
//    	osThreadSetPriority(thread_id, previous_priority);
//    }
////
////#endif



	return status;
}

static uint8_t run(DHT* dht, CelsiusAndHumidity* res)
{
	HAL_GPIO_WritePin(dht->m_GPIOx, dht->m_pin_id, GPIO_PIN_SET);

	__HAL_TIM_SET_COUNTER(dht->micro_clock,0);

	set_as_input(dht);

	while(__HAL_TIM_GET_COUNTER(dht->micro_clock) < 40){

		__NOP();
	}


	if(HAL_GPIO_ReadPin(dht->m_GPIOx, dht->m_pin_id) == GPIO_PIN_RESET){

		//around 80 us
		__HAL_TIM_SET_COUNTER(dht->micro_clock,0);
		while(HAL_GPIO_ReadPin(dht->m_GPIOx,dht->m_pin_id) == GPIO_PIN_RESET)
		{
			if(__HAL_TIM_GET_COUNTER(dht->micro_clock) > TIMEOUT_THRESHOLD)
			{
				return DHT_FAIL;
			}
		}

		//around 80 us
		__HAL_TIM_SET_COUNTER(dht->micro_clock,0);
		while(HAL_GPIO_ReadPin(dht->m_GPIOx,dht->m_pin_id) == GPIO_PIN_SET)
		{
			if(__HAL_TIM_GET_COUNTER(dht->micro_clock) > TIMEOUT_THRESHOLD)
			{
				return DHT_FAIL;
			}
		}

	}
	else{
		return DHT_FAIL;
	}


	return calc_bits(dht, res);
}



void print_CelsiusAndHumidity(CelsiusAndHumidity* res)
{
    if (res == NULL)
    {
        return;
    }

    printf("Temperature: %d.%d°C\r\n", res->tempature_integral, res->tempature_fractional);
    printf("Humidity: %d.%d%%\r\n", res->humidity_integral, res->humidity_fractional);
    printf("\r\n");
}

static uint8_t calc_bits(DHT* dht, CelsiusAndHumidity* res)
{
	uint8_t dht_bits[5] = {0};


	for(uint8_t i = 0; i < 5; ++i){
		uint8_t byte = 0;

		for (uint8_t j = 0; j < 8; ++j){
			__HAL_TIM_SET_COUNTER(dht->micro_clock,0);

			while(HAL_GPIO_ReadPin(dht->m_GPIOx,dht->m_pin_id) == GPIO_PIN_RESET){
				if(__HAL_TIM_GET_COUNTER(dht->micro_clock) > TIMEOUT_THRESHOLD)
				{

					return DHT_FAIL;
				}
			}

			__HAL_TIM_SET_COUNTER(dht->micro_clock,0);

			while(HAL_GPIO_ReadPin(dht->m_GPIOx, dht->m_pin_id) == GPIO_PIN_SET){
				if(__HAL_TIM_GET_COUNTER(dht->micro_clock) > TIMEOUT_THRESHOLD)
				{
					return DHT_FAIL;
				}
			}

			byte = byte << 1;
			byte |= __HAL_TIM_GET_COUNTER(dht->micro_clock) > 50 ? 1 : 0;
		}

		dht_bits[i] = byte;
	}

	res->humidity_integral = dht_bits[0];
	res->humidity_fractional = dht_bits[1];
	res->tempature_integral = dht_bits[2];
	res->tempature_fractional = dht_bits[3];




	if(dht_bits[0] + dht_bits[1] + dht_bits[2] + dht_bits[3] != dht_bits[4])
	{
		return DHT_FAIL;
	}

	return DHT_OK;

}

static uint8_t read_bit(DHT* dht)
{
	while(HAL_GPIO_ReadPin(dht->m_GPIOx,dht->m_pin_id) == GPIO_PIN_RESET){
		//__NOP();
	}

	__HAL_TIM_SET_COUNTER(dht->micro_clock,0);

	while(HAL_GPIO_ReadPin(dht->m_GPIOx,dht->m_pin_id) == GPIO_PIN_SET){
		//__NOP();
	}

	return  __HAL_TIM_GET_COUNTER(dht->micro_clock) > 50 ? 1 : 0;

}


static void set_as_input(DHT* dht)
{
	GPIO_InitTypeDef DHTinputMode = {0};
	DHTinputMode.Pin = dht->m_pin_id;
	DHTinputMode.Mode = GPIO_MODE_INPUT;
	DHTinputMode.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(dht->m_GPIOx, &DHTinputMode);
}

static void set_as_output(DHT* dht)
{

	GPIO_InitTypeDef DHToutputMode = {0};
	DHToutputMode.Pin = dht->m_pin_id;
	DHToutputMode.Mode = GPIO_MODE_OUTPUT_PP;
	DHToutputMode.Pull = GPIO_NOPULL;
	DHToutputMode.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(dht->m_GPIOx, &DHToutputMode);

}
