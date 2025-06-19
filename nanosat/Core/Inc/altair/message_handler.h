/**
 * @file message_handler.h
 * @brief Communication protocol implementation for the Altair system
 *
 * This module provides functions for handling communication messages between
 * the Altair system and external devices. It implements a packet-based protocol
 * for commands, responses, and data transmission. The module handles various
 * packet types for configuration updates, data requests, event logging, and
 * system status reporting.
 *
 * Created on: Apr 6, 2025
 * Author: 97254
 */

#ifndef INC_ALTAIR_MESSAGE_HANDLER_H_
#define INC_ALTAIR_MESSAGE_HANDLER_H_

#include <stdint.h>
#include "utils/send_queue.h"

#include "tasks/event_task.h"
#include "sensor_data.h"

/**
 * @brief End marker for all protocol packets
 *
 * This constant value marks the end of each protocol packet.
 * It is used for packet validation and synchronization.
 */
#define END_MARK 0x55

/**
 * @brief Protocol packet types enumeration
 *
 * Defines all valid packet types that can be sent or received
 * in the communication protocol. Each type represents a different
 * operation or data transfer.
 */
typedef enum {
    PACKET_TYPE_FORBIDDEN = 0x00,         /**< Reserved/invalid packet type */
    PACKET_TYPE_KEEP_ALIVE = 0x01,        /**< Keep-alive message with latest sensor data */
    PACKET_TYPE_GET_CLOCK = 0x02,         /**< Set system clock command */
    PACKET_TYPE_UPDATE_MIN_TEMP = 0x03,   /**< Update minimum temperature threshold */
    PACKET_TYPE_UPDATE_HUMIDITY = 0x04,   /**< Update humidity threshold */
    PACKET_TYPE_UPDATE_VOLTAGE = 0x05,    /**< Update voltage threshold */
    PACKET_TYPE_UPDATE_LIGHT = 0x06,      /**< Update light threshold */
    PACKET_TYPE_EVENT = 0x07,             /**< Event notification */
    PACKET_TYPE_ACK = 0x08,               /**< Acknowledge response */
    PACKET_TYPE_NACK = 0x09,              /**< Negative acknowledge response */
    PACKET_TYPE_UPDATE_MAX_TEMP = 0x0A,   /**< Update maximum temperature threshold */
    PACKET_TYPE_SEND_CLOCK = 0x10,        /**< Send current clock command */
    PACKET_TYPE_SENSOR_LOG = 0x11,        /**< Sensor log data packet */
    PACKET_TYPE_SENSOR_LOG_END = 0x12,    /**< End of sensor log transmission */
    PACKET_TYPE_REQUEST_SENSOR_LOG = 0x13,/**< Request sensor logs command */
    PACKET_TYPE_EVENT_LOG = 0x14,         /**< Event log data packet */
    PACKET_TYPE_EVENT_LOG_END = 0x15,     /**< End of event log transmission */
    PACKET_TYPE_REQUEST_EVENT_LOG = 0x16, /**< Request event logs command */
    PACKET_TYPE_REQUEST_GET_TIME = 0x17,  /**< Request current time */
    PACKET_TYPE_RESPONSE_SENT_TIME = 0x18,/**< Response with current time */
} PacketType;

/**
 * @brief Message packet structure
 *
 * Defines the structure of a communication packet in the protocol.
 * Each packet contains header information, payload data, and an end marker.
 */
typedef struct MessagePacket {
    uint8_t data_len;       /**< Total packet length in bytes including all fields */
    uint8_t packetType;     /**< Packet type from the PacketType enumeration */
    uint8_t m_respnse_id;   /**< Response ID for matching requests with responses */
    uint8_t checksum;       /**< Data integrity checksum */
    uint8_t buffer[128];    /**< Buffer for packet payload data */
    uint8_t end_mark;       /**< End marker for packet validation (should be END_MARK) */
} MessagePacket;

/**
 * @brief Process incoming messages
 *
 * Handles incoming messages based on packet type, executing the corresponding
 * actions and sending appropriate responses.
 *
 * @param send_data Pointer to the Queue instance for sending response messages
 * @param message Pointer to the received message buffer
 * @param len Length of the received message in bytes
 *
 * @note This function dispatches the message to appropriate handlers based on packet type.
 * It processes commands for setting clock, updating threshold values, and retrieving logs.
 */
void altair_message_handler(Queue* send_data, uint8_t const * message, uint8_t len);

/**
 * @brief Send a message packet
 *
 * Formats and enqueues a message packet for transmission.
 *
 * @param queue Pointer to the Queue instance for sending messages
 * @param message Pointer to the MessagePacket to be sent
 *
 * @note This function handles the proper formatting of the message before
 * enqueueing it for transmission. It constructs the complete packet
 * with all required fields.
 */
void send_message(Queue* queue, MessagePacket* message);

/**
 * @brief Send an event notification packet
 *
 * Creates and sends a packet containing event data.
 *
 * @param event_data Pointer to the EventData structure with event information
 * @param queue Pointer to the Queue instance for sending messages
 *
 * @note This function is used to notify external systems about events
 * that have occurred in the Altair system.
 */
void send_event_packet(EventData* event_data, Queue* queue);

/**
 * @brief Send a keep-alive packet
 *
 * Creates and sends a packet containing the latest sensor data.
 * This is typically used as a periodic status update.
 *
 * @param queue Pointer to the Queue instance for sending messages
 *
 * @note The keep-alive packet includes the current values from all sensors
 * along with a timestamp. It uses global data from g_latest_sensor_data.
 */
void send_keep_alive_packet(Queue* queue);

/**
 * @brief Send a time request packet
 *
 * Creates and sends a packet requesting the current time from an external system.
 *
 * @param queue Pointer to the Queue instance for sending messages
 *
 * @note This function is used to request time synchronization from
 * an external time source.
 */
void send_time_request(Queue* queue);

#endif /* INC_ALTAIR_MESSAGE_HANDLER_H_ */
