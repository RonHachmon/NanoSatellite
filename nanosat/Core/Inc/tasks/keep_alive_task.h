/**
 * @file keep_alive_task.h
 * @brief Periodic system heartbeat mechanism
 *
 * This module provides a periodic heartbeat mechanism for the Altair system,
 * ensuring that external components are regularly notified of the system's
 * operational status. The keep-alive task sends periodic status messages
 * containing the latest sensor readings and system state.
 *
 * These messages serve multiple purposes:
 * 1. Confirming that the system is functioning properly
 * 2. Providing regular updates of the current sensor values
 * 3. Notifying connected clients of the current system mode
 * 4. Maintaining communication links that might time out without activity
 *
 * Created on: Apr 7, 2025
 * Author: 97254
 */

#ifndef INC_TASKS_KEEP_ALIVE_TASK_H_
#define INC_TASKS_KEEP_ALIVE_TASK_H_

/**
 * @brief Main keep-alive mechanism task function
 *
 * This function implements the FreeRTOS task that sends periodic keep-alive
 * messages containing the latest system state and sensor readings. It waits
 * for system initialization to complete before beginning to send messages.
 *
 * @param context Pointer to the transmit queue for sending keep-alive messages
 *
 * @note This function is designed to run as a FreeRTOS task and does not return.
 *       It sets the FLAG_KEEP_ALIVE event flag after each message is sent to
 *       notify other components that a keep-alive cycle has completed.
 */
void keep_alive_task(void* context);

#endif /* INC_TASKS_KEEP_ALIVE_TASK_H_ */
