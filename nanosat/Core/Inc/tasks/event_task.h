/**
 * @file event_task.h
 * @brief Event handling system for the Altair system
 *
 * This module provides structures, enumerations, and functions for managing
 * system events in the Altair platform. It handles event detection, logging,
 * and retrieval. The event system tracks state transitions, system resets,
 * and other significant operational events.
 *
 * Events are timestamped and stored persistently in the file system, allowing
 * for historical analysis of system behavior. The module also supports
 * retrieving events within specific time ranges.
 *
 * Created on: Apr 3, 2025
 * Author: 97254
 */

#ifndef INC_TASKS_EVENT_TASK_H_
#define INC_TASKS_EVENT_TASK_H_

#include <stdint.h>

/**
 * @brief System event types enumeration
 *
 * Defines all possible system events that can occur in the Altair system.
 * Events primarily represent mode transitions and system state changes.
 */
typedef enum {
    EVENT_OK_TO_ERROR = 0,   /**< Transition from normal to error mode */
    EVENT_ERROR_TO_OK,       /**< Transition from error to normal mode */
    EVENT_WD_RESET,          /**< Watchdog timer reset occurred */
    EVENT_INIT,              /**< System initialization completed */
    EVENT_OK_TO_SAFE,        /**< Transition from normal to safe mode */
    EVENT_SAFE_TO_ERROR,     /**< Transition from safe to error mode */
    EVENT_SAFE_TO_OK,        /**< Transition from safe to normal mode */
    EVENT_ERROR_TO_SAFE,     /**< Transition from error to safe mode */
} AltairEvent;

/**
 * @brief Event operation status codes
 *
 * Defines status codes that indicate the result of event-related operations,
 * especially those involving data extraction from storage.
 */
typedef enum {
    EVENT_STATUS_SUCCESS = 0,           /**< Operation completed successfully */
    EVENT_STATUS_INVALID_PARAMS = -1,   /**< Invalid parameters provided */
    EVENT_STATUS_FILE_ERROR = -2,       /**< Error accessing or reading files */
    EVENT_STATUS_PARTIAL_DATA = 1       /**< Some data was retrieved (not full max_entries) */
} EventDataExtractionStatus;

/**
 * @brief Event data structure
 *
 * This structure represents a single system event, including the event type
 * and when it occurred (timestamp).
 */
typedef struct EventData {
    uint32_t timestamp;   /**< Unix timestamp when the event occurred */
    AltairEvent event;    /**< Type of event that occurred */
} EventData;

/**
 * @brief Extract event data within a time range
 *
 * Retrieves events that occurred within the specified time range and
 * stores them in the provided buffer. This function allows for querying
 * historical event data.
 *
 * @param buffer Buffer to store the retrieved events
 * @param timestamp_start Start of the time range (inclusive)
 * @param timestamp_end End of the time range (inclusive)
 * @param max_entries Maximum number of events to retrieve
 * @param entries_read_out Pointer to store the actual number of events retrieved
 *
 * @return Status code indicating the result of the operation
 *
 * @note This function acquires a mutex to ensure thread-safe access to event data.
 *       It returns EVENT_STATUS_PARTIAL_DATA if fewer than max_entries were found
 *       in the specified time range.
 */
EventDataExtractionStatus extract_event_data_between_timestamp(
    EventData* buffer,
    uint32_t timestamp_start,
    uint32_t timestamp_end,
    uint8_t max_entries,
    uint8_t* entries_read_out
);

/**
 * @brief Main event processing task function
 *
 * This function implements the FreeRTOS task that handles system events.
 * It monitors the event queue, writes events to persistent storage,
 * and transmits event notifications.
 *
 * @param context Pointer to the transmit queue for sending event notifications
 *
 * @note This function is designed to run as a FreeRTOS task and does not return.
 *       It initializes the event filesystem and creates a mutex for thread-safe
 *       event storage operations.
 */
void event_task(void* context);

#endif /* INC_TASKS_EVENT_TASK_H_ */
