#ifndef PACKET_PARSER_HPP
#define PACKET_PARSER_HPP

#include <vector>
#include <cstdint>
#include <string>

namespace altair
{
/**
 * @brief Constants used for packet parsing and creation
 */
constexpr uint8_t END_MARK = 0x55;           ///< End marker for packets
constexpr uint8_t PACKET_HEADER_SIZE = 5;    ///< Size of packet header (data_len + type + id + checksum + end mark)

/**
 * @enum AltairModes
 * @brief Operating modes for the Altair satellite
 */
enum AltairModes
{
    ERROR_MODE = 0x01,    ///< Satellite is in error mode
    SAFE_MODE = 0x02,     ///< Satellite is in safe mode
    OK_MODE = 0x03,       ///< Satellite is in normal operating mode
};

/**
 * @struct SensorData
 * @brief Container for satellite sensor readings
 */
struct SensorData
{
    uint32_t timestamp;   ///< Unix timestamp when readings were taken
    uint8_t temp;         ///< Temperature reading in degrees Celsius
    uint8_t humid;        ///< Humidity reading in percentage
    uint8_t light;        ///< Light level in percentage
    AltairModes mode;     ///< Current operating mode
    float voltage;        ///< Battery voltage level
};

/**
 * @enum AltairEvent
 * @brief Events that can occur on the Altair satellite
 */
enum AltairEvent
{
    OK_TO_ERROR = 0,      ///< Transition from OK mode to error mode
    ERROR_TO_OK,          ///< Transition from error mode to OK mode
    WD_RESET,             ///< Watchdog timer reset event
    INIT,                 ///< Initialization event
    OK_TO_SAFE,           ///< Transition from OK mode to safe mode
    SAFE_TO_ERROR,        ///< Transition from safe mode to error mode
    SAFE_TO_OK,           ///< Transition from safe mode to OK mode
    ERROR_TO_SAFE,        ///< Transition from error mode to safe mode
};

/**
 * @enum ResponseType
 * @brief Types of communication packets between server and satellite
 */
enum ResponseType
{
    BEACON = 0x01,                ///< Regular beacon signal
    TIME_SEND = 0x02,             ///< Time update being sent to satellite
    UPDATE_MIN_TEMP = 0x03,       ///< Update minimum temperature threshold
    UPDATE_HUMIDITY = 0x04,       ///< Update humidity threshold
    UPDATE_VOLTAGE = 0x05,        ///< Update voltage threshold
    UPDATE_LIGHT = 0x06,          ///< Update light threshold
    EVENT = 0x07,                 ///< Event notification
    ACK = 0x08,                   ///< Acknowledgment of a command
    NACK = 0x09,                  ///< Negative acknowledgment of a command
    UPDATE_MAX_TEMP = 0x0A,       ///< Update maximum temperature threshold
    TIME_REQUEST = 0x10,          ///< Request for time update from satellite
    SENSOR_LOG = 0x11,            ///< Sensor log data
    TOTAL_LOGS = 0x12,            ///< Total number of logs available
    REQUEST_SENSOR_LOGS = 0x13,   ///< Request for sensor logs
    EVENT_LOG = 0x14,             ///< Event log data
    EVENT_LOG_END = 0x15,         ///< End of event log transmission
    REQUEST_EVENT_LOG = 0x16,     ///< Request for event logs
    REQUEST_CURRENT_TIME = 0x17,  ///< Request for current satellite time
    RESPONSE_CURRENT_TIME = 0x18, ///< Response with current satellite time
    UNKNOWN = 0xFF                ///< Default for unknown types
};

/**
 * @struct MessagePacket
 * @brief Structure for communication packets
 */
struct MessagePacket
{
    uint8_t data_len;          ///< Length of data in packet
    uint8_t packetType;        ///< Type of packet (see ResponseType enum)
    uint8_t m_respnse_id;      ///< ID for tracking request/response pairs
    uint8_t checksum;          ///< Packet checksum
    uint8_t buffer[128];       ///< Buffer for packet payload
    uint8_t end_mark;          ///< End marker (should be END_MARK constant)
};

/**
 * @struct EventData
 * @brief Container for satellite event information
 */
struct EventData
{
    uint32_t timestamp;     ///< Unix timestamp when event occurred
    AltairEvent event;      ///< Type of event
};

/**
 * @class PacketParser
 * @brief Handles parsing and formatting of satellite communication packets
 * 
 * This class provides functionality to parse raw byte data from the satellite
 * into structured data objects, and to convert structured data back into string
 * representations for display or logging.
 */
class PacketParser
{
public:
    /**
     * @brief Default constructor
     */
    PacketParser() = default;
    
    /**
     * @brief Default destructor
     */
    ~PacketParser() = default;
    
    //---------------------------------------------------------------------
    // Parse methods
    //---------------------------------------------------------------------
    
    /**
     * @brief Extract the response type from a raw packet
     * @param response Vector containing the raw packet data
     * @return The ResponseType value from the packet
     */
    ResponseType parse_response_type(const std::vector<uint8_t>& response) const;
    
    /**
     * @brief Parse sensor data from a raw packet
     * @param response Vector containing the raw packet data
     * @param data Reference to a SensorData object to populate with parsed data
     */
    void parse_sensor_data(const std::vector<uint8_t>& response, SensorData& data) const;
    
    /**
     * @brief Parse event data from a raw packet
     * @param response Vector containing the raw packet data
     * @param event_data Reference to an EventData object to populate with parsed data
     */
    void parse_event_data(const std::vector<uint8_t>& response, EventData& event_data) const;
    
    //---------------------------------------------------------------------
    // String conversion methods
    //---------------------------------------------------------------------
    
    /**
     * @brief Convert sensor data to a formatted string
     * @param data The SensorData object to convert
     * @return Formatted string representation of the sensor data
     */
    std::string sensor_data_to_string(const SensorData& data) const;
    
    /**
     * @brief Convert event data to a formatted string
     * @param data The EventData object to convert
     * @return Formatted string representation of the event data
     */
    std::string event_data_to_string(const EventData& data) const;
    
    /**
     * @brief Format a Unix timestamp into a human-readable string
     * @param timestamp The Unix timestamp to format
     * @return Formatted date-time string
     */
    std::string format_timestamp(time_t timestamp) const;
    
    //---------------------------------------------------------------------
    // Message creation
    //---------------------------------------------------------------------
    
    /**
     * @brief Create a new message packet with the specified type and response ID
     * @param type The type of message to create
     * @param response_id The response ID to use
     * @return A MessagePacket structure initialized with the specified parameters
     */
    static MessagePacket create_message_packet(ResponseType type, uint8_t response_id);
    
    //---------------------------------------------------------------------
    // Utility methods
    //---------------------------------------------------------------------
    
    /**
     * @brief Check if a response packet is valid
     * @param response Vector containing the raw packet data
     * @return true if the packet is valid, false otherwise
     */
    bool is_valid_response(const std::vector<uint8_t>& response) const;
    
    /**
     * @brief Print beacon data to standard output
     * @param data The SensorData to print
     */
    void print_beacon_data(const SensorData& data) const;
    
    /**
     * @brief Print sensor data to standard output
     * @param data The SensorData to print
     */
    void print_sensor_data(const SensorData& data) const;
    
    /**
     * @brief Print event data to standard output
     * @param data The EventData to print
     */
    void print_event(const EventData& data) const;
    
private:
    /**
     * @brief Convert an AltairMode enum value to a string
     * @param mode The mode to convert
     * @return String representation of the mode
     */
    std::string get_mode_string(AltairModes mode) const;
    
    /**
     * @brief Convert an AltairEvent enum value to a string
     * @param event The event to convert
     * @return String representation of the event
     */
    std::string get_event_string(AltairEvent event) const;
};

} // namespace altair

#endif // PACKET_PARSER_HPP
