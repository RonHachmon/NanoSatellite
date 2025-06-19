/*
 * flash_task.c
 *
 *  Created on: Apr 6, 2025
 *      Author: 97254
 */

#include "tasks/flash_task.h"
#include "sync_globals.h"
#include "main.h"

//typedef struct CollectorSetting{
//	uint8_t delay;
//	uint8_t min_temp;
//	uint8_t max_temp;
//	uint8_t min_humidity;
//	uint8_t min_light;
//	float safe_voltage;
//
//}CollectorSetting;

//typedef struct CurrentSetting {
//    uint16_t delay;
//    uint16_t min_temp;
//    uint16_t max_temp;
//    uint16_t min_humidity;
//    uint16_t min_light;
//    float safe_voltage;
//} CurrentSetting;

#define DELAY_ADDRESS        0x08080000
#define MIN_TEMP_ADDRESS     0x08080008
#define MAX_TEMP_ADDRESS     0x08080010
#define MIN_HUMDITY_ADDRESS  0x08080018
#define MIN_LIGHT_ADDRESS    0x08080020
#define SAFE_VOLTAGE_ADDRESS 0x08080028

#define SECOND 1

static CollectorSetting current_setting = {0};

static void update_flash(void);
static void erase_flash_page(uint32_t bank, uint32_t page);

void reset_flash()
{
    uint8_t buffer[8];

    // Default values
    uint8_t delay =  SECOND * 6 ;
    uint8_t min_temp = 15;
    uint8_t max_temp = 30;
    uint8_t min_humidity = 20;
    uint8_t min_light = 70;
    float safe_voltage = 2.2;

    current_setting.delay = delay;
    current_setting.min_temp = min_temp;
    current_setting.max_temp = max_temp;
    current_setting.min_humidity = min_humidity;
    current_setting.min_light = min_light;
    current_setting.safe_voltage = safe_voltage;

    // Unlock Flash for writing
    HAL_FLASH_Unlock();

    // Erase flash page (bank 2, page 0)
    erase_flash_page(FLASH_BANK_2, 0x08080000);

    // Write all values into flash
    update_flash();

    // Lock Flash after writing
    HAL_FLASH_Lock();
}

void read_settings(CollectorSetting* collector_settings)
{
    if (collector_settings == NULL){
        return;
    }

    static uint8_t buffer[8];

    // Read delay
    memcpy(buffer, (void*)DELAY_ADDRESS, sizeof(buffer));
    memcpy(&collector_settings->delay, buffer, sizeof(collector_settings->delay));

    // Read min_temp
    memcpy(buffer, (void*)MIN_TEMP_ADDRESS, sizeof(buffer));
    memcpy(&collector_settings->min_temp, buffer, sizeof(collector_settings->min_temp));

    // Read max_temp
    memcpy(buffer, (void*)MAX_TEMP_ADDRESS, sizeof(buffer));
    memcpy(&collector_settings->max_temp, buffer, sizeof(collector_settings->max_temp));

    // Read min_humidity
    memcpy(buffer, (void*)MIN_HUMDITY_ADDRESS, sizeof(buffer));
    memcpy(&collector_settings->min_humidity, buffer, sizeof(collector_settings->min_humidity));

    // Read min_light
    memcpy(buffer, (void*)MIN_LIGHT_ADDRESS, sizeof(buffer));
    memcpy(&collector_settings->min_light, buffer, sizeof(collector_settings->min_light));

    // Read safe_voltage
    memcpy(buffer, (void*)SAFE_VOLTAGE_ADDRESS, sizeof(buffer));
    memcpy(&collector_settings->safe_voltage, buffer, sizeof(collector_settings->safe_voltage));
}


void flash_task(void* context)
{
    UpdateSetting setting;

    printf("flash task start \r\n");
    while(1)
    {
        memset(setting.buffer, 0, 8);

        osStatus_t status = osMessageQueueGet(g_flah_limits_change, &setting, 0, HAL_MAX_DELAY);

        if (status == osOK) {
            printf("run flash update \r\n");

            switch (setting.update_attribute) {
                case UPDATE_HUMIDITY_LIMIT:
                    current_setting.min_humidity = *(uint16_t*)setting.buffer;
                    break;
                case UPDATE_VOLTAGE_LIMIT:
                    current_setting.safe_voltage = *(float*)setting.buffer;
                    break;
                case UPDATE_LIGHT_LIMIT:
                    current_setting.min_light = *(uint16_t*)setting.buffer;
                    break;
                case UPDATE_MIN_TEMP_LIMIT:
                    current_setting.min_temp = *(uint16_t*)setting.buffer;
                    break;
                case UPDATE_MAX_TEMP_LIMIT:
                    current_setting.max_temp = *(uint16_t*)setting.buffer;
                    break;
                default:
                    // Handle invalid update_attribute if needed
                    break;
            }
            update_flash();
        }
    }
}

static void update_flash(void)
{
    uint8_t buffer[8];
    HAL_FLASH_Unlock();

    // Erase flash page (bank 2, page 0)
    erase_flash_page(FLASH_BANK_2, 0x08080000);

    HAL_StatusTypeDef status = HAL_ERROR;

    // Write delay
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &current_setting.delay, sizeof(current_setting.delay));
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, DELAY_ADDRESS, *(uint64_t*)buffer);

    // Write min_temp
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &current_setting.min_temp, sizeof(current_setting.min_temp));
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, MIN_TEMP_ADDRESS, *(uint64_t*)buffer);

    // Write max_temp
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &current_setting.max_temp, sizeof(current_setting.max_temp));
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, MAX_TEMP_ADDRESS, *(uint64_t*)buffer);

    // Write min_humidity
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &current_setting.min_humidity, sizeof(current_setting.min_humidity));
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, MIN_HUMDITY_ADDRESS, *(uint64_t*)buffer);

    // Write min_light
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &current_setting.min_light, sizeof(current_setting.min_light));
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, MIN_LIGHT_ADDRESS, *(uint64_t*)buffer);

    // Write safe_voltage
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &current_setting.safe_voltage, sizeof(current_setting.safe_voltage));
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, SAFE_VOLTAGE_ADDRESS, *(uint64_t*)buffer);

    if (status != HAL_OK) {
        printf("flash write error \r\n");
    }

    HAL_FLASH_Lock();
}

static void erase_flash_page(uint32_t bank, uint32_t page)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t errorStatus = 0;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Banks = bank;
    erase.Page = page;
    erase.NbPages = 1;

    HAL_StatusTypeDef res = HAL_FLASHEx_Erase(&erase, &errorStatus);
    if (res != HAL_OK) {
        printf("Flash erase failed! Bank: %lu, Page: %lu\r\n", bank, page);
        HAL_FLASH_Lock();
        return;
    }
}
