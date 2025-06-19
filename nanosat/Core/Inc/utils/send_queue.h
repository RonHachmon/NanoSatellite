/**
 * @file send_queue.h
 * @brief Circular buffer implementation for byte queue management
 *
 * This module provides a fixed-size circular buffer implementation designed
 * for managing queues of bytes. It supports basic queue operations like
 * enqueueing multiple bytes and retrieving single bytes from the queue.
 * The implementation uses a circular buffer to efficiently manage memory.
 *
 * Created on: Apr 6, 2025
 * Author: 97254
 */

#ifndef INC_UTILS_SEND_QUEUE_H_
#define INC_UTILS_SEND_QUEUE_H_

#include <stdint.h>

/**
 * @brief Maximum size of the queue in bytes
 *
 * Defines the maximum number of bytes that can be stored in the queue.
 * The queue implementation will not accept new data if it would exceed
 * this capacity.
 */
#define QUEUE_SIZE 256

/**
 * @brief Queue data structure
 *
 * Implements a circular buffer for byte data with tracking of
 * head, tail, and current number of items.
 *
 * @note The implementation uses modulo arithmetic to wrap around
 * the buffer when reaching its end.
 */
typedef struct Queue {
    uint8_t m_que[QUEUE_SIZE];  /**< Buffer storage for the queue data */
    uint16_t m_head;            /**< Index of the first element in the queue */
    uint16_t m_tail;            /**< Index where the next element will be inserted */
    uint16_t m_nItems;          /**< Current number of bytes in the queue */
} Queue;

/**
 * @brief Creates and initializes a new queue
 *
 * @return Pointer to a statically allocated Queue structure initialized with empty state
 *
 * @note This function returns a pointer to a static Queue instance.
 * Only one queue instance is supported by this implementation.
 */
Queue* Queue_create();

/**
 * @brief Adds multiple bytes to the queue
 *
 * @param queue Pointer to the Queue instance
 * @param data Pointer to the array of bytes to be added
 * @param size Number of bytes to add from the data array
 *
 * @return 1 if the operation was successful, 0 if the queue doesn't have enough space
 *
 * @note The function will not add any bytes if the requested size would exceed
 * the queue's capacity. It's an all-or-nothing operation.
 */
uint8_t Queue_enque(Queue *queue, uint8_t *data, uint16_t size);

/**
 * @brief Retrieves and removes a single byte from the queue
 *
 * @param queue Pointer to the Queue instance
 *
 * @return The byte value from the head of the queue if available,
 * or 0 if the queue is empty
 *
 * @note The function does not distinguish between a legitimate 0 value
 * and an empty queue condition. External checks using Queue_size()
 * should be performed if this distinction is important.
 */
uint8_t Queue_getChar(Queue *queue);

/**
 * @brief Gets the current number of bytes in the queue
 *
 * @param queue Pointer to the Queue instance
 *
 * @return Current number of bytes stored in the queue
 *
 * @note This function can be used to check if the queue is empty
 * before calling Queue_getChar() or to determine available space
 * before Queue_enque().
 */
uint16_t Queue_size(Queue *queue);

#endif /* INC_UTILS_SEND_QUEUE_H_ */
