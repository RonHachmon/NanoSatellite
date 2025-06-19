/**
 * @file collector_task.h
 * @brief Sensor data collection task and configuration
 *
 * This module provides structures and functions for managing sensor data
 * collection in the Altair system. It defines the configuration settings
 * for sensor thresholds and implements the collection task that monitors
 * environmental conditions.
 *
 * The collector task periodically samples various sensors (temperature,
 * humidity, light, voltage), evaluates their values against configurable
 * thresholds, and manages system modes based on sensor readings.
 *
 * Created on: Apr 2, 2025
 * Author: 97254
 */

#ifndef INC_COLLECTOR_TASK_H_
#define INC_COLLECTOR_TASK_H_

#include "main.h"

/**
 * @brief Configuration settings for the collector task
 *
 * This structure defines the configurable parameters used by the collector task
 * to evaluate sensor readings and determine system operating modes. It includes
 * threshold values for different sensors and timing parameters.
 */
typedef struct CollectorSetting {
    uint8_t delay;        /**< Data collection interval in seconds */
    uint8_t min_temp;     /**< Minimum acceptable temperature in degrees Celsius */
    uint8_t max_temp;     /**< Maximum acceptable temperature in degrees Celsius */
    uint8_t min_humidity; /**< Minimum acceptable humidity percentage */
    uint8_t min_light;    /**< Minimum acceptable light level percentage */
    float safe_voltage;   /**< Minimum voltage level for safe operation */
} CollectorSetting;

/**
 * @brief Main sensor collection task function
 *
 * This function implements the FreeRTOS task that handles sensor data collection.
 * It periodically reads sensor values, processes them according to the configured
 * thresholds, updates the system mode, and sends data to other system components.
 *
 * @param context Pointer to the initial CollectorSetting configuration
 *
 * @note This function is designed to run as a FreeRTOS task and does not return.
 *       It responds to configuration updates through message queues and handles
 *       mode transitions by generating appropriate events.
 */
void collection_task(void* context);

#endif /* INC_COLLECTOR_TASK_H_ */
