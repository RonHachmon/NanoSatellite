/*
 * uart_task.c
 *
 *  Created on: Apr 5, 2025
 *      Author: 97254
 */


#include "tasks/uart_task.h"
#include "usart.h"
#include "cmsis_os2.h"
#include "sync_globals.h"
#include "mod_led.h"


#define ENTER '\r'

static uint8_t uart_buffer[1];

static Queue recieve_queue;

static osSemaphoreId_t commands;


int _write(int file, char* ptr, int len);

static void wait_for_data_if_all_queues_empty(TransmitContext* ctx);

static void transmit_queue_data(Queue* queue);
static void transmit_queue_event(Queue* queue);



void UART_send_message(uint8_t const * buffer, uint8_t size)
{
	HAL_UART_Transmit(&huart2, buffer, size, HAL_MAX_DELAY);
}

void init_uart()
{
	Queue_create(&recieve_queue);
	commands = osSemaphoreNew(8, 0, NULL);

	HAL_UART_Receive_IT(&huart2, uart_buffer, sizeof(uart_buffer));
}


void transmit_task(void* context)
{
    TransmitContext* ctx = (TransmitContext*) context;

    while (1)
    {
        wait_for_data_if_all_queues_empty(ctx);

        if (Queue_size(&ctx->high_priority) != 0) {
        	transmit_queue_data(&ctx->high_priority);
        }
        else if (Queue_size(&ctx->medium_priority) != 0) {
        	transmit_queue_data(&ctx->medium_priority);
        }
        else if (Queue_size(&ctx->low_priority) != 0) {
        	transmit_queue_data(&ctx->low_priority);
        }
    }
}

static void wait_for_data_if_all_queues_empty(TransmitContext* ctx)
{
    if (Queue_size(&ctx->high_priority) == 0 &&
        Queue_size(&ctx->medium_priority) == 0 &&
        Queue_size(&ctx->low_priority) == 0) {
        osEventFlagsWait(g_evtID, FLAG_KEEP_ALIVE | FLAG_REPORTS | FLAG_EVENT, osFlagsWaitAny, HAL_MAX_DELAY);
    }
}

static void transmit_queue_data(Queue* queue)
{
	uint8_t len = Queue_getChar(queue);
	HAL_UART_Transmit(&huart2, &len, 1, HAL_MAX_DELAY);

	uint8_t current_char;
	for(uint8_t i = 0; i < len -1; ++i){
		current_char = Queue_getChar(queue);
		HAL_UART_Transmit(&huart2, &current_char, 1, HAL_MAX_DELAY);
	}

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	static uint8_t message_len;
	static uint8_t message_start = 0;
	static uint8_t message_idx = 0;



    Queue_enque(&recieve_queue, uart_buffer , 1);
    if(message_start == 0 )
    {
    	message_len = uart_buffer[0];
    	message_start = 1;

    }
    ++message_idx;


    if (message_idx == message_len){
    	message_start = 0;
    	message_idx = 0;
    	osSemaphoreRelease(commands);
    }

    HAL_UART_Receive_IT(&huart2, uart_buffer, sizeof(uart_buffer));
}




void recieve_task(void* context)
{

	RecieveContext* recieve_context =  (RecieveContext*)  context;

	uint8_t command[256];
	uint8_t command_index = 0;
	uint8_t current_char;
	uint8_t message_len = 0;

	while(1)
	{

		osSemaphoreAcquire(commands, HAL_MAX_DELAY);
		command_index = 0;

		message_len = Queue_getChar(&recieve_queue);
	    command[command_index] = message_len;
	    command_index++;
	    --message_len;

		while(message_len > 0)
		{
		    current_char = Queue_getChar(&recieve_queue);
		    command[command_index] = current_char;
		    ++command_index;
		    --message_len;
		}

		recieve_context->message_handler(recieve_context->response_queue, command,command_index);
	}
}

int _write(int file, char* ptr, int len)
{
	HAL_UART_Transmit(&huart2, (uint8_t*) ptr, len, HAL_MAX_DELAY);

	return len;
}


