/*
 * message_handler.c
 *
 * Created on: Apr 6, 2025
 * Author: 97254
 */

#include <string.h>
#include "altair/message_handler.h"
#include "DateTime.h"
#include "sync_globals.h"
#include "utils/send_queue.h"
#include "tasks/init_altair_task.h"
#include "tasks/collector_task.h"
#include "tasks/logger_task.h"
#include "tasks/flash_task.h"
#include "tasks/uart_task.h"

/* Constants */
#define HEADER_LEN 5
#define MAX_LOGS 10

/* Forward declarations of static functions */
static void send_event_logs(Queue* send_data, EventData* event_data, uint8_t len, uint8_t response_id);
static void send_sensor_logs(Queue* send_data, SensorData* sensor_data, uint8_t len, uint8_t response_id, uint8_t more_data);
static void send_time(Queue* send_data, uint8_t response_id);
static void wrap_message(uint8_t const * message, MessagePacket* packet);
static void send_ack(Queue* send_data, uint8_t response_id);
static void send_nack(Queue* send_data, uint8_t response_id);

/* Handlers for each packet type */
static void handle_get_clock(Queue* send_data, MessagePacket* packet);
static void handle_update_min_temp(Queue* send_data, MessagePacket* packet);
static void handle_update_max_temp(Queue* send_data, MessagePacket* packet);
static void handle_update_humidity(Queue* send_data, MessagePacket* packet);
static void handle_update_voltage(Queue* send_data, MessagePacket* packet);
static void handle_update_light(Queue* send_data, MessagePacket* packet);
static void handle_request_sensor_log(Queue* send_data, MessagePacket* packet);
static void handle_request_event_log(Queue* send_data, MessagePacket* packet);
static void handle_request_get_time(Queue* send_data, MessagePacket* packet);

/* Helper functions */
static void update_setting_and_notify(Queue* send_data, uint8_t* buffer, uint8_t size, uint8_t attribute, uint8_t response_id);
static int validate_percentage_value(uint8_t value);



/* Main message handler function */
void altair_message_handler(Queue* send_data, uint8_t const * message, uint8_t len) {
    MessagePacket packet;
    wrap_message(message, &packet);

    switch (packet.packetType) {
        case PACKET_TYPE_GET_CLOCK:
            handle_get_clock(send_data, &packet);
            break;
        case PACKET_TYPE_UPDATE_MIN_TEMP:
            handle_update_min_temp(send_data, &packet);
            break;
        case PACKET_TYPE_UPDATE_MAX_TEMP:
            handle_update_max_temp(send_data, &packet);
            break;
        case PACKET_TYPE_UPDATE_HUMIDITY:
            handle_update_humidity(send_data, &packet);
            break;
        case PACKET_TYPE_UPDATE_VOLTAGE:
            handle_update_voltage(send_data, &packet);
            break;
        case PACKET_TYPE_UPDATE_LIGHT:
            handle_update_light(send_data, &packet);
            break;
        case PACKET_TYPE_REQUEST_SENSOR_LOG:
            handle_request_sensor_log(send_data, &packet);
            break;
        case PACKET_TYPE_REQUEST_EVENT_LOG:
            handle_request_event_log(send_data, &packet);
            break;
        case PACKET_TYPE_REQUEST_GET_TIME:
            handle_request_get_time(send_data, &packet);
            break;
        default:
            /* Handle unknown or unsupported packet type */
            break;
    }
}

/* Implementation of send_keep_alive_packet function */
void send_keep_alive_packet(Queue* queue) {
    if (queue == NULL) {
        return;
    }

    MessagePacket packet;

    packet.packetType = PACKET_TYPE_KEEP_ALIVE;
    packet.data_len = 17;
    packet.m_respnse_id = 0xFF;
    packet.checksum = 8;
    packet.end_mark = END_MARK;

    packet.buffer[0] = g_latest_sensor_data.temp;
    packet.buffer[1] = g_latest_sensor_data.humid;
    packet.buffer[2] = g_latest_sensor_data.light;
    packet.buffer[3] = g_latest_sensor_data.mode;

    uint8_t* voltage = (uint8_t*)&g_latest_sensor_data.volage;
    memcpy(&packet.buffer[4], voltage, sizeof(float));

    uint8_t* timestamp = (uint8_t*)&g_latest_sensor_data.timestamp;
    memcpy(&packet.buffer[8], timestamp, sizeof(uint32_t));

    send_message(queue, &packet);
}

/* Implementation of send_event_packet function */
void send_event_packet(EventData* event_data, Queue* queue) {

    if (queue == NULL || event_data == NULL) {
        return;
    }

    MessagePacket packet;

    packet.packetType = PACKET_TYPE_EVENT;
    packet.data_len = 10;
    packet.checksum = 8;
    packet.m_respnse_id = 0xFF;
    packet.end_mark = END_MARK;

    packet.buffer[0] = event_data->event;

    uint8_t* timestamp = (uint8_t*)&event_data->timestamp;
    memcpy(&packet.buffer[1], timestamp, sizeof(uint32_t));

    send_message(queue, &packet);
}

/* Implementation of send_message function */
void send_message(Queue* queue, MessagePacket* message) {
    if (queue == NULL || message == NULL) {
        return;
    }

    uint8_t writeBuf[128];

    writeBuf[0] = message->data_len;
    writeBuf[1] = message->packetType;
    writeBuf[2] = message->m_respnse_id;
    writeBuf[3] = message->checksum;

    if (message->data_len - HEADER_LEN > 0) {
        memcpy(&writeBuf[4], message->buffer, message->data_len - HEADER_LEN);
    }

    writeBuf[message->data_len - 1] = END_MARK;
    Queue_enque(queue, writeBuf, message->data_len);
}

/* Implementation of send_ack function */
static void send_ack(Queue* send_data, uint8_t response_id) {
    MessagePacket messagePacket;
    messagePacket.packetType = PACKET_TYPE_ACK;
    messagePacket.data_len = HEADER_LEN;
    messagePacket.checksum = 0;
    messagePacket.m_respnse_id = response_id;
    messagePacket.end_mark = END_MARK;
    send_message(send_data, &messagePacket);
}

/* Implementation of send_nack function */
static void send_nack(Queue* send_data, uint8_t response_id) {
    MessagePacket messagePacket;
    messagePacket.packetType = PACKET_TYPE_NACK;
    messagePacket.data_len = HEADER_LEN;
    messagePacket.checksum = 0;
    messagePacket.m_respnse_id = response_id;
    messagePacket.end_mark = END_MARK;
    send_message(send_data, &messagePacket);
}

/* Handler implementations */
static void handle_get_clock(Queue* send_data, MessagePacket* packet) {
    uint32_t timestamp;
    DateTime datetime;
    memcpy(&timestamp, packet->buffer, sizeof(uint32_t));

    parse_timestamp(timestamp, &datetime);
    RTC_SetDateTime(&datetime);
    osEventFlagsSet(g_evtID, FLAG_SET_TIME);
    send_ack(send_data, packet->m_respnse_id);
}

static void handle_update_min_temp(Queue* send_data, MessagePacket* packet) {
    if (!validate_percentage_value(packet->buffer[0])) {
        send_nack(send_data, packet->m_respnse_id);
        return;
    }
    update_setting_and_notify(send_data, packet->buffer, sizeof(uint8_t),
                             UPDATE_MIN_TEMP_LIMIT, packet->m_respnse_id);
}

static void handle_update_max_temp(Queue* send_data, MessagePacket* packet) {
    if (!validate_percentage_value(packet->buffer[0])) {
        send_nack(send_data, packet->m_respnse_id);
        return;
    }
    update_setting_and_notify(send_data, packet->buffer, sizeof(uint8_t),
                             UPDATE_MAX_TEMP_LIMIT, packet->m_respnse_id);
}

static void handle_update_humidity(Queue* send_data, MessagePacket* packet) {
    if (!validate_percentage_value(packet->buffer[0])) {
        send_nack(send_data, packet->m_respnse_id);
        return;
    }
    update_setting_and_notify(send_data, packet->buffer, sizeof(uint8_t),
                             UPDATE_HUMIDITY_LIMIT, packet->m_respnse_id);
}

static void handle_update_voltage(Queue* send_data, MessagePacket* packet) {
    update_setting_and_notify(send_data, packet->buffer, sizeof(float),
                             UPDATE_VOLTAGE_LIMIT, packet->m_respnse_id);
}

static void handle_update_light(Queue* send_data, MessagePacket* packet) {
    if (!validate_percentage_value(packet->buffer[0])) {
        send_nack(send_data, packet->m_respnse_id);
        return;
    }
    update_setting_and_notify(send_data, packet->buffer, sizeof(uint8_t),
                             UPDATE_LIGHT_LIMIT, packet->m_respnse_id);
}

static void handle_request_sensor_log(Queue* send_data, MessagePacket* packet) {
    SensorData sensor_data[MAX_LOGS];
    uint32_t start_timestamp, end_timestamp;
    uint8_t total_logs;

    memcpy(&start_timestamp, packet->buffer, sizeof(uint32_t));
    memcpy(&end_timestamp, packet->buffer + sizeof(uint32_t), sizeof(uint32_t));

    DataExtractionStatus res = extract_data_between_timestamp(
        sensor_data, start_timestamp, end_timestamp, MAX_LOGS, &total_logs);

    if (res >= 0) {
        printf("EXTRACTED data %u \r\n", total_logs);
        send_sensor_logs(send_data, sensor_data, total_logs, packet->m_respnse_id, res);
    } else {
        send_nack(send_data, packet->m_respnse_id);
    }
}

static void handle_request_event_log(Queue* send_data, MessagePacket* packet) {
    EventData event_data[MAX_LOGS];
    uint32_t event_start_timestamp, event_end_timestamp;
    uint8_t total_event_logs;

    memcpy(&event_start_timestamp, packet->buffer, sizeof(uint32_t));
    memcpy(&event_end_timestamp, packet->buffer + sizeof(uint32_t), sizeof(uint32_t));

    EventDataExtractionStatus event_res = extract_event_data_between_timestamp(
        event_data, event_start_timestamp, event_end_timestamp, MAX_LOGS, &total_event_logs);

    if (event_res >= 0) {
        printf("EXTRACTED Events data %u \r\n", total_event_logs);
        send_event_logs(send_data, event_data, total_event_logs, packet->m_respnse_id);
    } else {
        send_nack(send_data, packet->m_respnse_id);
    }
}

static void handle_request_get_time(Queue* send_data, MessagePacket* packet) {
    send_time(send_data, packet->m_respnse_id);
}

/* Helper function implementations */
static void update_setting_and_notify(Queue* send_data, uint8_t* buffer, uint8_t size,
                                     uint8_t attribute, uint8_t response_id) {
    UpdateSetting update_setting = {0};
    memcpy(update_setting.buffer, buffer, size);
    update_setting.update_attribute = attribute;

    osMessageQueuePut(g_flah_limits_change, &update_setting, 0, 200);
    osMessageQueuePut(g_sensor_limits_change, &update_setting, 0, HAL_MAX_DELAY);
    send_ack(send_data, response_id);
}

static int validate_percentage_value(uint8_t value) {
    return (value <= 100);
}

/* Implementation of send_time function */
static void send_time(Queue* send_data, uint8_t response_id) {
    MessagePacket messagePacket;

    messagePacket.packetType = PACKET_TYPE_RESPONSE_SENT_TIME;
    messagePacket.data_len = 4 + HEADER_LEN + 2;
    messagePacket.checksum = 8;
    messagePacket.m_respnse_id = response_id;
    messagePacket.end_mark = END_MARK;

    uint32_t current_time = get_timestamp();
    uint8_t* timestamp = (uint8_t*)&current_time;
    memcpy(&messagePacket.buffer[0], timestamp, sizeof(uint32_t));

    send_message(send_data, &messagePacket);
}

/* Implementation of wrap_message function */
static void wrap_message(uint8_t const * message, MessagePacket* packet) {
    packet->data_len = message[0];
    packet->packetType = message[1];
    packet->m_respnse_id = message[2];
    packet->checksum = message[3];

    memcpy(packet->buffer, &message[4], packet->data_len - HEADER_LEN);
    packet->end_mark = message[packet->data_len - 1];
}

/* Implementation of send_event_logs function */
static void send_event_logs(Queue* send_data, EventData* event_data, uint8_t len, uint8_t response_id) {
    MessagePacket messagePacket;

    messagePacket.packetType = PACKET_TYPE_EVENT_LOG;
    messagePacket.data_len = 11;
    messagePacket.checksum = 8;
    messagePacket.m_respnse_id = response_id;
    messagePacket.end_mark = END_MARK;

    for (uint8_t i = 0; i < len; ++i) {
        messagePacket.buffer[0] = event_data[i].event;

        uint8_t* timestamp = (uint8_t*)&event_data->timestamp;
        memcpy(&messagePacket.buffer[1], timestamp, sizeof(uint32_t));

        send_message(send_data, &messagePacket);
    }

    /* Send end marker packet */
    messagePacket.packetType = PACKET_TYPE_EVENT_LOG_END;
    messagePacket.data_len = HEADER_LEN;
    messagePacket.checksum = 0;
    messagePacket.m_respnse_id = response_id;
    messagePacket.end_mark = END_MARK;
    send_message(send_data, &messagePacket);
}

/* Implementation of send_sensor_logs function */
static void send_sensor_logs(Queue* send_data, SensorData* sensor_data, uint8_t len,
                            uint8_t response_id, uint8_t more_data) {
    MessagePacket messagePacket;

    messagePacket.packetType = PACKET_TYPE_SENSOR_LOG;
    messagePacket.data_len = 17;
    messagePacket.checksum = 8;
    messagePacket.m_respnse_id = response_id;
    messagePacket.end_mark = END_MARK;

    for (uint8_t i = 0; i < len; ++i) {
        messagePacket.buffer[0] = sensor_data[i].temp;
        messagePacket.buffer[1] = sensor_data[i].humid;
        messagePacket.buffer[2] = sensor_data[i].light;
        messagePacket.buffer[3] = sensor_data[i].mode;

        uint8_t* voltage = (uint8_t*)&sensor_data[i].volage;
        memcpy(&messagePacket.buffer[4], voltage, sizeof(float));

        uint8_t* timestamp = (uint8_t*)&sensor_data[i].timestamp;
        memcpy(&messagePacket.buffer[8], timestamp, sizeof(uint32_t));

        send_message(send_data, &messagePacket);
    }

    /* Send end marker packet */
    messagePacket.packetType = PACKET_TYPE_SENSOR_LOG_END;
    messagePacket.data_len = HEADER_LEN + 1;
    messagePacket.checksum = 0;
    messagePacket.m_respnse_id = response_id;
    messagePacket.end_mark = END_MARK;
    messagePacket.buffer[0] = more_data ? 1 : 0;

    send_message(send_data, &messagePacket);
}


