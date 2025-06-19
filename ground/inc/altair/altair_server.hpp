#ifndef ALTAIR_SERVER_HPP
#define ALTAIR_SERVER_HPP

#include <memory>
#include <functional>
#include <unordered_map>
#include "connection.hpp"
#include "id_generator.hpp"
#include "tcp_server.hpp"
#include "packet_parser.hpp"
#include "server_data_manager.hpp"

namespace altair {

/**
 * @class AltairServer
 * @brief Main server class for the Altair satellite communication system
 * 
 * The AltairServer provides an interface for communicating with the Altair satellite.
 * It handles incoming responses from the satellite and outgoing requests from clients.
 * The server operates both through a serial connection with the satellite and
 * a TCP server for external client connections.
 */
class AltairServer {
public:
    /**
     * @brief Constructs an AltairServer with the given connection
     * @param connection A unique pointer to a Connection object for satellite communication
     */
    explicit AltairServer(std::unique_ptr<Connection> connection);

    /**
     * @brief Virtual destructor with default implementation
     */
    ~AltairServer() = default;

    /**
     * @brief Deleted copy constructor to prevent copying
     */
    AltairServer(const AltairServer&) = delete;

    /**
     * @brief Deleted assignment operator to prevent copying
     */
    AltairServer& operator=(const AltairServer&) = delete;

    /**
     * @brief Starts the main listening loop for satellite communications
     * 
     * This method continuously reads data from the satellite connection,
     * processes responses, and dispatches them to appropriate handlers.
     */
    void listen();

private:
    /**
     * @typedef ResponseHandler
     * @brief Function type for handling specific response types from the satellite
     */
    using ResponseHandler = std::function<void(std::vector<uint8_t>&, uint8_t)>;
    
    /**
     * @brief Initializes the response handler map
     * 
     * Maps each ResponseType to its corresponding handler method.
     * Called during construction.
     */
    void init_response_handlers();
    
    /**
     * @brief Main handler for incoming response data from the satellite
     * @param response Vector of bytes containing the response data
     * 
     * Dispatches the response to the appropriate handler based on its type.
     */
    void handle_response(std::vector<uint8_t>& response);
    
    /**
     * @brief Handles client requests received through the TCP server
     * @param message The message received from the client
     * @param client A shared pointer to the client session
     * 
     * Parses the command and executes the appropriate action.
     */
    void handle_request(const std::string& message, std::shared_ptr<altair::ClientSession> client);
    
    /**
     * @brief Creates a message packet with the specified type and response ID
     * @param type The type of message to create
     * @param response_id The response ID to use for tracking
     * @return A MessagePacket structure ready to be populated with data
     */
    MessagePacket create_message_packet(ResponseType type, uint8_t response_id);
    
    /**
     * @brief Sends a message packet to the Altair satellite
     * @param message_packet The packet to send
     * 
     * Serializes the packet and sends it through the connection.
     */
    void send_packet_to_altair(MessagePacket& message_packet);
    
    /**
     * @brief Sends a value of any type to the satellite
     * @tparam T The type of value to send
     * @param type The message type to use
     * @param value The value to send
     * 
     * Creates a message packet, adds the value, and sends it to the satellite.
     */
    template<typename T>
    void send_value(ResponseType type, const T& value);

    //----------------------------------------------------------------------
    // Response handler methods
    //----------------------------------------------------------------------
    
    /**
     * @brief Handles TIME_REQUEST responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_time_request(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles SENSOR_LOG responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_sensor_log(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles SENSOR_LOG_END responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_sensor_log_end(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles ACK responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_ack(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles NACK responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_nack(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles EVENT responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_event(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles EVENT_LOG responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_event_log(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles EVENT_LOG_END responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_event_log_end(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles RESPONSE_CURRENT_TIME responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_response_current_time(std::vector<uint8_t>& response, uint8_t responseId);
    
    /**
     * @brief Handles BEACON responses
     * @param response The response data
     * @param responseId The response identifier
     */
    void handle_beacon(std::vector<uint8_t>& response, uint8_t responseId);

    //----------------------------------------------------------------------
    // Request handler methods
    //----------------------------------------------------------------------
    
    /**
     * @brief Sends a custom time to the satellite
     * @param custom_time The Unix timestamp to set
     */
    void send_custom_time(uint32_t custom_time);
    
    /**
     * @brief Sends the current system time to the satellite
     */
    void send_current_time();
    
    /**
     * @brief Updates the maximum temperature setting on the satellite
     * @param max_temp The new maximum temperature
     */
    void update_max_temp(uint8_t max_temp);
    
    /**
     * @brief Updates the minimum temperature setting on the satellite
     * @param min_temp The new minimum temperature
     */
    void update_min_temp(uint8_t min_temp);
    
    /**
     * @brief Updates the humidity setting on the satellite
     * @param humidity The new humidity value
     */
    void update_humidity(uint8_t humidity);
    
    /**
     * @brief Updates the voltage setting on the satellite
     * @param voltage The new voltage value
     */
    void update_voltage(float voltage);
    
    /**
     * @brief Updates the light setting on the satellite
     * @param light The new light value
     */
    void update_light(uint8_t light);
    
    /**
     * @brief Requests sensor data within a time range
     * @param start The start timestamp
     * @param end The end timestamp
     * @param client The client session to send the results to
     */
    void get_sensor_in_range(uint32_t start, uint32_t end, std::shared_ptr<altair::ClientSession> client);
    
    /**
     * @brief Requests event data within a time range
     * @param start The start timestamp
     * @param end The end timestamp
     * @param client The client session to send the results to
     */
    void get_event_in_range(uint32_t start, uint32_t end, std::shared_ptr<altair::ClientSession> client);
    
    /**
     * @brief Requests the current time from the satellite
     * @param client The client session to send the result to
     */
    void get_current_time(std::shared_ptr<altair::ClientSession> client);

private:
    /**
     * Connection to the Altair satellite
     */
    std::unique_ptr<Connection> m_connection;
    
    /**
     * Latest sensor data received from the satellite
     */
    SensorData m_latest_data;
    
    /**
     * ID generator for tracking requests
     */
    IDGenerator& m_id_generator;
    
    /**
     * TCP server for client connections
     */
    TcpServer m_tcp_server;
    
    /**
     * Parser for satellite communication packets
     */
    PacketParser m_packet_parser;
    
    /**
     * Map of request IDs to client sessions for tracking responses
     */
    std::unordered_map<uint8_t, std::shared_ptr<altair::ClientSession>> m_request_clients;
    
    /**
     * Storage for historical sensor data
     */
    ServerDataManager m_sensor_data_manager;
    
    /**
     * Map of response types to handler functions
     */
    std::unordered_map<ResponseType, ResponseHandler> m_response_handlers;
};

} // namespace altair

#endif // ALTAIR_SERVER_HPP