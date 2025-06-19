/*
 * send_queue.c
 *
 *  Created on: Apr 6, 2025
 *      Author: 97254
 */



#include <string.h>

#include "utils/send_queue.h"

Queue* Queue_create() {
    static Queue queue = { .m_head = 0, .m_tail = 0, .m_nItems = 0 };
    return &queue;
}


uint8_t Queue_enque(Queue* queue, uint8_t* data, uint16_t size)
{
    if (queue->m_nItems + size >= QUEUE_SIZE) {
        return 0;
    }

    for (uint16_t i = 0; i < size; i++) {
        queue->m_que[queue->m_tail] = data[i];
        queue->m_tail = (queue->m_tail + 1) % QUEUE_SIZE;
    }

    queue->m_nItems += size;
    return 1;
}


uint8_t Queue_getChar(Queue* queue)
{

	if (queue->m_nItems == 0) {
        return 0;
    }

    char value = queue->m_que[queue->m_head];
    queue->m_head = (queue->m_head + 1) % QUEUE_SIZE;
    queue->m_nItems--;

    return value;
}


uint16_t Queue_size(Queue* queue)
{
    return queue->m_nItems;
}

