/**
 * @file uart_task.h
 * @brief UART communication interface for the Altair system
 *
 * This module provides functions and structures for managing serial
 * communication via UART in the Altair system. It implements a priority-based
 * transmit mechanism and a command reception system that processes incoming
 * messages according to the Altair protocol.
 *
 * The module uses queues to manage message transmission with different
 * priority levels, and it provides mechanisms for receiving and parsing
 * incoming messages. It also supports registering message handler functions
 * to process received commands.
 *
 * Created on: Apr 5, 2025
 * Author: 97254
 */

#ifndef INC_TASKS_UART_TASK_H_
#define INC_TASKS_UART_TASK_H_

#include "main.h"
#include "utils/send_queue.h"

/**
 * @brief Message handler function pointer type
 *
 * This type defines the function signature for message handler callbacks.
 * Functions of this type are called to process received messages.
 *
 * @param response_queue Pointer to the queue for sending responses
 * @param message Pointer to the received message buffer
 * @param len Length of the received message in bytes
 */
typedef void (*MessageHandler)(Queue* response_queue, uint8_t const *message, uint8_t len);

/**
 * @brief Transmit context structure
 *
 * This structure contains the queues used for transmitting messages
 * with different priority levels. The transmit task processes messages
 * from these queues in order of priority.
 */
typedef struct TransmitContext {
    Queue high_priority;    /**< Queue for high-priority messages */
    Queue medium_priority;  /**< Queue for medium-priority messages */
    Queue low_priority;     /**< Queue for low-priority messages */
} TransmitContext;

/**
 * @brief Receive context structure
 *
 * This structure contains the parameters needed by the receive task
 * to process incoming messages, including a queue for responses and
 * a handler function to process messages.
 */
typedef struct RecieveContext {
    Queue* response_queue;        /**< Queue for sending responses */
    MessageHandler message_handler;  /**< Function to handle received messages */
} RecieveContext;

/**
 * @brief Initialize the UART interface
 *
 * Initializes the UART hardware, creates the necessary queues and
 * synchronization objects, and starts the receive interrupt. This function
 * should be called once during system startup before any UART operations.
 */
void init_uart();

/**
 * @brief Transmit task function
 *
 * This function implements the FreeRTOS task that processes outgoing
 * messages. It monitors the priority queues and transmits messages
 * according to their priority, handling high-priority messages first.
 *
 * @param context Pointer to the TransmitContext structure
 *
 * @note This function is designed to run as a FreeRTOS task and does not return.
 */
void transmit_task(void* context);

/**
 * @brief Receive task function
 *
 * This function implements the FreeRTOS task that processes incoming
 * messages. It monitors the receive buffer, assembles complete messages,
 * and passes them to the registered message handler function.
 *
 * @param context Pointer to the RecieveContext structure
 *
 * @note This function is designed to run as a FreeRTOS task and does not return.
 */
void recieve_task(void* context);

/**
 * @brief Send a message via UART
 *
 * Transmits the specified buffer content via the UART interface.
 * This function is typically used for direct transmission without
 * going through the priority queue system.
 *
 * @param buffer Pointer to the data to be transmitted
 * @param size Number of bytes to transmit
 */
void UART_send_message(uint8_t const * buffer, uint8_t size);

/**
 * @brief UART receive complete callback
 *
 * This function is called by the HAL UART driver when a character is
 * received via UART. It processes the received character and manages
 * the assembly of complete messages.
 *
 * @param huart Pointer to the UART handle structure
 *
 * @note This function is typically registered with the HAL UART driver
 *       and should not be called directly.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif /* INC_TASKS_UART_TASK_H_ */
