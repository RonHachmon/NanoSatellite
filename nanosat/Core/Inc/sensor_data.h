/*
 * sensor_data.h
 *
 *  Created on: Apr 1, 2025
 *      Author: 97254
 */

#ifndef SENSOR_DATA_H_
#define SENSOR_DATA_H_

#include <stdint.h>

#define UNINTILIZED_MODE 0
#define ERROR_MODE 1
#define SAFE_MODE 2
#define OK_MODE 3

typedef struct SensorData{
    uint32_t timestamp;
    uint8_t temp;
    uint8_t humid;
    uint8_t light;
    uint8_t mode;
    float volage;

} SensorData;

extern SensorData g_latest_sensor_data;


#endif //SENSOR_DATA_H_
