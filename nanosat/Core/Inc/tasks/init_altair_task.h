/**
 * @file init_altair_task.h
 * @brief System initialization task for the Altair platform
 *
 * This module provides the entry point function for initializing the entire
 * Altair system. It handles the orderly startup sequence, creating all
 * required tasks, initializing system components, and establishing the
 * communication infrastructure.
 *
 * The initialization process includes setting up hardware peripherals,
 * initializing the file system, creating system queues, synchronizing the
 * system time, and launching all the system's operational tasks. Once
 * initialization is complete, the init task self-terminates to free resources.
 *
 * Created on: Mar 31, 2025
 * Author: 97254
 */

#ifndef INC_INIT_ALTAIR_TASK_H_
#define INC_INIT_ALTAIR_TASK_H_

/**
 * @brief Main system initialization function
 *
 * This function orchestrates the entire system initialization sequence.
 * It performs the following operations in order:
 * 1. Initializes system components (UART, file system, global queues)
 * 2. Creates the receive and transmit communication tasks
 * 3. Waits for time synchronization from an external source
 * 4. Creates all operational tasks (event, logger, collector, etc.)
 * 5. Self-terminates after successful initialization
 *
 * @param context Task context parameter (unused)
 *
 * @note This function is designed to run as a FreeRTOS task and self-terminates
 *       upon successful completion of the initialization sequence.
 */
void init_altair(void* context);

#endif /* INC_INIT_ALTAIR_TASK_H_ */
