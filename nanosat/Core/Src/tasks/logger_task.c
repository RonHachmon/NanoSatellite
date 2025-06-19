/*
 * logger.c
 *
 *  Created on: Apr 1, 2025
 *      Author: 97254
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ff_gen_drv.h"

#include "utils/send_queue.h"
#include "tasks/logger_task.h"
#include "DateTime.h"
#include "dht.h"
#include "sync_globals.h"

#include "sensor_data.h"

#define STR_EQUEAL 0


//#define LOGGER_DEBUG 1

#define TRUE 1
#define FALSE 0

#define MAX_DATA_FILES 7
#define FILENAME_SIZE 9
#define SENSOR_DIR_PATH     "0:/sensor"

#define MAX_WHOLE_FILENAME_LEN 64

#define MAX_WRITE_RETRIES           7


static char file_names[MAX_DATA_FILES][FILENAME_SIZE] = {0};


static osMutexId_t file_mutexes[MAX_DATA_FILES];


static void extract_file_name_from_timestamp(char* file_name, uint32_t timestamp);
static FRESULT create_or_open_file(FIL* fil, const char* file_path, uint8_t new_file);
static FRESULT write_sensor_data_to_file(FIL* fil, SensorData* data);
static uint8_t manage_file_switch(uint8_t* file_index, const char* current_file_name);
static int8_t get_file_index(const char* file_name);

static uint8_t read_data_from_file(SensorData* buffer,
    uint32_t timestamp_start,
    uint32_t timestamp_end,
    uint8_t file_index,
    uint8_t max_entries
);



void init_logger() {
    static FATFS FatFs;
    static uint8_t init_flag = 0;
    osDelay(1000);

    if (init_flag == 0) {
        FRESULT fres;
        fres = f_mount(&FatFs, "/", 1);
        while (fres != FR_OK) {
            printf("f_mount error (%i)\r\n", fres);
            osDelay(1000);
            fres = f_mount(&FatFs, "/", 1);
        }
    }
    init_flag = 1;

    uint8_t mutexes_flag = 7;

    for (uint8_t i = 0; i < MAX_DATA_FILES; ++i) {
        file_mutexes[i] = osMutexNew(NULL);

        if (file_mutexes[i] == NULL) {
            --mutexes_flag;
        }
    }

#ifdef LOGGER_DEBUG
    printf("mutexes_flag created %u \r\n", mutexes_flag);
    printf("f_mount Success \r\n");
#endif

}



DataExtractionStatus extract_data_between_timestamp(
    SensorData* buffer,
    uint32_t timestamp_start,
    uint32_t timestamp_end,
    uint8_t max_entries,
    uint8_t* entries_read_out
) {


	if(buffer == NULL || entries_read_out == NULL){
		return STATUS_NULL_ERROR;
	}


    if (timestamp_end < timestamp_start || max_entries == 0 || NULL == buffer || NULL == entries_read_out) {
        return STATUS_INVALID_PARAMS;
    }

    char first_file[FILENAME_SIZE];
    char second_file[FILENAME_SIZE];

    extract_file_name_from_timestamp(first_file, timestamp_start);
    extract_file_name_from_timestamp(second_file, timestamp_end);

    int8_t idx_start = get_file_index(first_file);
    int8_t idx_end = get_file_index(second_file);

    if (idx_start < 0 || idx_end < 0) {
        return STATUS_NO_SUCH_FILE;
    }

    uint8_t entries_read = 0;

    if (idx_start == idx_end) {
        entries_read = read_data_from_file(buffer, timestamp_start, timestamp_end, idx_start, max_entries);
    } else {
        // Read from start file
        uint8_t entries_1 = read_data_from_file(buffer, timestamp_start, UINT32_MAX, idx_start, max_entries);
        entries_read += entries_1;

        if (entries_read < max_entries) {
            // Read from end file
            uint8_t entries_2 = read_data_from_file(
                buffer + entries_1,
                0,
                timestamp_end,
                idx_end,
                max_entries - entries_1
            );
            entries_read += entries_2;
        }
    }

    // Store entries read in output parameter
    *entries_read_out = entries_read;

    // Return appropriate status
    if (entries_read < max_entries) {
        return STATUS_PARTIAL_DATA;
    }

    return STATUS_SUCCESS;
}

void logger_beacon_task(void* context) {
    FIL fil;
    FRESULT fres;
    uint8_t file_index = 0;

    char current_file_name[FILENAME_SIZE];
    char whole_file_path[32];

    fres = f_mkdir(SENSOR_DIR_PATH);
    while (fres != FR_OK && fres != FR_EXIST) {
        #ifdef LOGGER_DEBUG
        printf("f_mkdir error (%i)\r\n", fres);
        #endif
        osDelay(500);
        fres = f_mkdir(SENSOR_DIR_PATH);
    }

    #ifdef LOGGER_DEBUG
    printf("Open sensor directory\r\n");
    #endif

    while (1) {
        SensorData data;
        osStatus_t status = osMessageQueueGet(g_sensor_queue, &data, 0, HAL_MAX_DELAY);
        uint8_t new_file;

        if (status == osOK) {
            // Extract file name based on the timestamp
            extract_file_name_from_timestamp(current_file_name, data.timestamp);

            #ifdef LOGGER_DEBUG
            printf("file name extracted (%s)\r\n", current_file_name);
            #endif

            // Switch to the new file if needed
            new_file = manage_file_switch(&file_index, current_file_name);

            // Build full file path
            snprintf(whole_file_path, sizeof(whole_file_path), "%s/%s", SENSOR_DIR_PATH, file_names[file_index]);

            // Open or create the file
            osMutexAcquire(file_mutexes[file_index], osWaitForever);

            fres = create_or_open_file(&fil, whole_file_path, new_file);
            if (fres != FR_OK) {
                #ifdef LOGGER_DEBUG
                printf("failed to open log file to write (%i)\r\n", fres);
                #endif
                continue;  // Skip this iteration if we fail to open the file
            }
            #ifdef LOGGER_DEBUG
            printf("f_open Success open %s to write\r\n", whole_file_path);
            #endif

            // Write the sensor data to the file
            fres = write_sensor_data_to_file(&fil, &data);
            if (fres != FR_OK) {
                f_close(&fil);  // Close file on error
                continue;
            }
            #ifdef LOGGER_DEBUG
            printf("Write Success\r\n");
            #endif

            // Sync and close the file
            f_sync(&fil);
            f_close(&fil);
            osMutexRelease(file_mutexes[file_index]);
        } else {
            #ifdef LOGGER_DEBUG
            printf("failed to get data, Status: %d\r\n", status);
            #endif
        }
    }
}

//#define FA_READ              0x01    // Open file for reading. This flag enables reading from the file.
//#define FA_WRITE             0x02    // Open file for writing. This flag enables writing to the file.
//#define FA_OPEN_EXISTING     0x00    // Open file only if it already exists. If the file does not exist, the open operation fails.
//#define FA_CREATE_NEW        0x04    // Create a new file. If the file already exists, the open operation fails.
//#define FA_CREATE_ALWAYS     0x08    // Create a new file or overwrite an existing file. If the file already exists, it is erased and recreated.
//#define FA_OPEN_ALWAYS       0x10    // Open a file if it exists. If the file does not exist, it will be created.
//#define FA_OPEN_APPEND       0x30    // Open the file for appending. Data will be written at the end of the file without modifying the existing content.

// Function to create or open a file, with retry logic
static FRESULT create_or_open_file(FIL* fil, const char* file_path, uint8_t new_file) {
    FRESULT fres;
    uint8_t retry_count = 0;
    BYTE mode;
    if(new_file){
    	mode = FA_CREATE_ALWAYS | FA_WRITE;
    }
    else{
    	mode = FA_OPEN_APPEND | FA_WRITE ;
    }

#ifdef LOGGER_DEBUG
    printf("trying to open %s \r\n", file_path);
#endif


    while ((fres = f_open(fil, file_path, mode)) != FR_OK) {
        if (++retry_count > MAX_WRITE_RETRIES) {
            return fres; // Return error after max retries
        }
        osDelay(500);  // Wait and retry
    }
    return FR_OK;
}


static FRESULT write_sensor_data_to_file(FIL* fil, SensorData* data) {
    FRESULT fres;
    UINT bytesWrote;
    uint8_t retry_count = 0;

    while ((fres = f_write(fil, data, sizeof(SensorData), &bytesWrote)) != FR_OK) {
        if (++retry_count > MAX_WRITE_RETRIES) {
            return fres; // Return error after max retries
        }
        osDelay(500);  // Wait and retry
    }
    return FR_OK;
}

// Function to manage file switching
static uint8_t manage_file_switch(uint8_t* file_index, const char* current_file_name) {
    uint8_t new_file = FALSE;

    if (strcmp(file_names[*file_index], current_file_name) != STR_EQUEAL) {
        new_file = TRUE;

        if (file_names[*file_index][0] == '\0') {
            strcpy(file_names[*file_index], current_file_name);
        } else {

            *file_index = (*file_index + 1) % MAX_DATA_FILES;
            if (file_names[*file_index][0] != '\0') {
                osMutexAcquire(file_mutexes[*file_index], osWaitForever);

                // Delete the previous file if it's going to be overwritten
                f_unlink(file_names[*file_index]);
                osMutexRelease(file_mutexes[*file_index]);
            }

            // Update file name and index
            strcpy(file_names[*file_index], current_file_name);
        }
    }

    return new_file;
}


static void extract_file_name_from_timestamp(char* file_name, uint32_t timestamp)
{
    char whole_time[50];
    DateTime datetime;

    parse_timestamp(timestamp, &datetime);
    RTC_DateTimeToString((char*) whole_time, &datetime);

    strncpy(file_name, whole_time, 8);
    file_name[8] = '\0';
}




static int8_t get_file_index(const char* file_name)
{
    for (int i = 0; i < MAX_DATA_FILES; ++i) {
        if (strcmp(file_names[i], file_name) == 0) {
            return i;
        }
    }
    return -1;
}

static uint8_t read_data_from_file(
    SensorData* buffer,
    uint32_t timestamp_start,
    uint32_t timestamp_end,
    uint8_t file_index,
    uint8_t max_entries
) {
    FRESULT fres;
    FIL fil;
    char full_path[MAX_WHOLE_FILENAME_LEN];
    UINT bytesRead;
    SensorData data;
    uint32_t total_bytes_read = 0;
    uint8_t retry_count = 0;


    snprintf(full_path, sizeof(full_path), "%s/%s", SENSOR_DIR_PATH, file_names[file_index]);

    osMutexAcquire(file_mutexes[file_index], osWaitForever);

    while ((fres = f_open(&fil, full_path, FA_READ | FA_OPEN_EXISTING)) != FR_OK) {
        if (++retry_count > MAX_WRITE_RETRIES) {
            osMutexRelease(file_mutexes[file_index]);
            return 0;
        }
        osDelay(500);
    }

    printf("open for read \r\n");

    while (f_read(&fil, &data, sizeof(SensorData), &bytesRead) == FR_OK && bytesRead == sizeof(SensorData)) {
        if (data.timestamp > timestamp_end) {
            break; // Optimization: data is sorted
        }
        if (data.timestamp >= timestamp_start) {
            buffer[total_bytes_read / sizeof(SensorData)] = data;
            total_bytes_read += bytesRead;

            if (total_bytes_read / sizeof(SensorData) >= max_entries) {
                break;
            }
        }
    }


    f_close(&fil);
    osMutexRelease(file_mutexes[file_index]);

    return total_bytes_read / sizeof(SensorData);
}


