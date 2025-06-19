/**
 * @file logger_task.h
 * @brief Data logging system for the Altair platform
 *
 * This module provides functionality for persistent storage and retrieval
 * of sensor data in the Altair system. It implements a file-based logging
 * system that organizes data by date, with automatic file rotation and
 * management capabilities.
 *
 * The logger system handles writing sensor readings to storage, maintaining
 * multiple log files for different time periods, and extracting historical
 * data based on timestamp ranges. It provides thread-safe access to the
 * logging functionality through appropriate synchronization mechanisms.
 *
 * Created on: Apr 1, 2025
 * Author: 97254
 */

#ifndef INC_LOGGER_TASK_H_
#define INC_LOGGER_TASK_H_

#include "sensor_data.h"

/**
 * @brief Status codes for data extraction operations
 *
 * Defines status codes that indicate the result of data extraction
 * operations from the logging system.
 */
typedef enum {
    STATUS_SUCCESS = 0,           /**< Operation completed successfully */
    STATUS_INVALID_PARAMS = -1,   /**< Invalid parameters provided */
    STATUS_FILE_ERROR = -2,       /**< Error accessing or reading files */
    STATUS_NO_SUCH_FILE = -3,     /**< Requested file does not exist */
    STATUS_NULL_ERROR = -4,       /**< Null pointer provided as parameter */
    STATUS_PARTIAL_DATA = 1       /**< Some data was retrieved (not full max_entries) */
} DataExtractionStatus;

/**
 * @brief Initialize the logging system
 *
 * Initializes the file system and creates the necessary mutexes for
 * thread-safe logging operations. This function should be called once
 * during system startup before any logging operations are performed.
 */
void init_logger();

/**
 * @brief Main logger task function
 *
 * This function implements the FreeRTOS task that processes incoming
 * sensor data and writes it to persistent storage. It monitors a message
 * queue for new sensor readings, determines the appropriate file to use,
 * and handles file rotation and management.
 *
 * @param context Task context parameter (unused)
 *
 * @note This function is designed to run as a FreeRTOS task and does not return.
 */
void logger_beacon_task(void* context);

/**
 * @brief Extract sensor data within a time range
 *
 * Retrieves sensor data that was recorded within the specified time range
 * and stores it in the provided buffer. This function allows for querying
 * historical sensor readings for analysis or reporting.
 *
 * @param buffer Buffer to store the retrieved sensor data
 * @param timestamp_start Start of the time range (inclusive)
 * @param timestamp_end End of the time range (inclusive)
 * @param max_entries Maximum number of entries to retrieve
 * @param entries_read_out Pointer to store the actual number of entries retrieved
 *
 * @return Status code indicating the result of the operation
 *
 * @note This function handles the case where data spans multiple files by
 *       reading from all relevant files within the time range.
 */
DataExtractionStatus extract_data_between_timestamp(
    SensorData* buffer,
    uint32_t timestamp_start,
    uint32_t timestamp_end,
    uint8_t max_entries,
    uint8_t* entries_read_out
);

#endif /* INC_LOGGER_TASK_H_ */
