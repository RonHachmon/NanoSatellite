/*
 * event_task.c
 *
 * Created on: Apr 3, 2025
 * Author: 97254
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ff_gen_drv.h"
#include "sync_globals.h"

#include "tasks/event_task.h"
#include "utils/send_queue.h"
#include "altair/message_handler.h"

static osMutexId_t event_mutex;

#define MAX_WRITE_RETRIES 3
#define EVENT_FILE_PATH "0:/events/event"
#define EVENT_DIR_PATH "0:/events"

// Forward declarations of static methods
static FRESULT open_file_with_retry(FIL* fil, const char* path, BYTE mode, uint8_t max_retries, uint32_t delay_ms);
static FRESULT create_directory(const char* path, uint32_t delay_ms);
static FRESULT initialize_event_filesystem(void);
static FRESULT write_event_to_file(EventData* data, Queue* transmit_queue);

EventDataExtractionStatus extract_event_data_between_timestamp(
    EventData* buffer,
    uint32_t timestamp_start,
    uint32_t timestamp_end,
    uint8_t max_entries,
    uint8_t* entries_read_out
) {
    if (timestamp_end < timestamp_start || max_entries == 0 || NULL == buffer || NULL == entries_read_out) {
        return EVENT_STATUS_INVALID_PARAMS;
    }

    osMutexAcquire(event_mutex, osWaitForever);

    FIL fil;
    FRESULT fres;
    UINT bytesRead;
    uint8_t entries_read = 0;
    EventData data;

    fres = open_file_with_retry(&fil, EVENT_FILE_PATH, FA_READ | FA_OPEN_EXISTING, MAX_WRITE_RETRIES, 500);
    if (fres != FR_OK) {
        osMutexRelease(event_mutex);
        return EVENT_STATUS_FILE_ERROR;
    }

    while (f_read(&fil, &data, sizeof(EventData), &bytesRead) == FR_OK && bytesRead == sizeof(EventData)) {
        if (data.timestamp > timestamp_end) {
            break;
        }

        if (data.timestamp >= timestamp_start) {
            buffer[entries_read] = data;
            entries_read++;

            if (entries_read >= max_entries) {
                break;
            }
        }
    }

    f_close(&fil);
    osMutexRelease(event_mutex);

    *entries_read_out = entries_read;

    // Return appropriate status
    if (entries_read < max_entries) {
        return EVENT_STATUS_PARTIAL_DATA;
    }

    return EVENT_STATUS_SUCCESS;
}

void event_task(void* context) {
    Queue* transmit_queue = (Queue*)context;

    initialize_event_filesystem();
    event_mutex = osMutexNew(NULL);

    while(1) {
        EventData data;
        osStatus_t status;

        status = osMessageQueueGet(g_event_queue, &data, 0, HAL_MAX_DELAY);

        if(status == osOK) {
            write_event_to_file(&data, transmit_queue);
        } else {
            printf("failed to get data, Status: %d\r\n", status);
        }
    }
}


static FRESULT open_file_with_retry(FIL* fil, const char* path, BYTE mode, uint8_t max_retries, uint32_t delay_ms) {
    FRESULT fres;
    uint8_t retry_count = 0;

    while ((fres = f_open(fil, path, mode)) != FR_OK) {
        if (++retry_count > max_retries) {
            printf("f_open error for %s (%i)\r\n", path, fres);
            return fres;
        }
        osDelay(delay_ms);
    }

    return FR_OK;
}

static FRESULT create_directory(const char* path, uint32_t delay_ms) {
    FRESULT fres;

    fres = f_mkdir(path);
    while (fres != FR_OK && fres != FR_EXIST) {
        printf("f_mkdir error for %s (%i)\r\n", path, fres);
        osDelay(delay_ms);
        fres = f_mkdir(path);
    }

    return fres;
}

static FRESULT initialize_event_filesystem(void) {
    FRESULT fres;
    FIL fil;

    fres = create_directory(EVENT_DIR_PATH, 100);
    if (fres != FR_OK && fres != FR_EXIST) {
        return fres;
    }

    printf("Open event directory\r\n");

    fres = f_open(&fil, EVENT_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    while (fres != FR_OK) {
        printf("f_open error event file (%i)\r\n", fres);
        osDelay(100);
        fres = f_open(&fil, EVENT_FILE_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    }

    f_close(&fil);
    printf("Open event file\r\n");

    return FR_OK;
}

static FRESULT write_event_to_file(EventData* data, Queue* transmit_queue) {
    FIL fil;
    FRESULT fres;
    UINT bytesWrote;

    osMutexAcquire(event_mutex, osWaitForever);

    fres = open_file_with_retry(&fil, EVENT_FILE_PATH, FA_OPEN_APPEND | FA_WRITE, MAX_WRITE_RETRIES, 100);
    if (fres != FR_OK) {
        osMutexRelease(event_mutex);
        return fres;
    }

    fres = f_write(&fil, data, sizeof(EventData), &bytesWrote);

    if (fres == FR_OK) {
        send_event_packet(data, transmit_queue);
        osEventFlagsSet(g_evtID, FLAG_EVENT);
        printf("wrote event to File And queue \r\n");
    } else {
        printf("f_write error (%i)\r\n", fres);
    }

    f_sync(&fil);
    f_close(&fil);
    osMutexRelease(event_mutex);

    return fres;
}
