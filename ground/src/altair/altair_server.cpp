#include "altair_server.hpp"
#include <chrono>
#include <string>
#include <iostream>
#include <sstream>

namespace altair {

AltairServer::AltairServer(std::unique_ptr<Connection> connection):
m_connection(std::move(connection)),
m_latest_data(),
m_id_generator(IDGenerator::getInstance()),
m_tcp_server(4444, 10),
m_sensor_data_manager()
{

    init_response_handlers();
    
    m_tcp_server.setMessageHandler([this](const std::string& message, std::shared_ptr<altair::ClientSession> client) {
        this->handle_request(message, client);
    });
    
    m_tcp_server.start();
}

void AltairServer::handle_response(std::vector<uint8_t>& response)
{
    if(response.size() < PACKET_HEADER_SIZE) {
        std::cout << "Invalid response size!" << std::endl;
        return;
    }

    if(response.size() == 9) {
        std::vector<uint8_t> new_response;
        new_response.push_back(10);
        for(size_t i = 0; i < response.size(); ++i) {
            new_response.push_back(response[i]);
        }
        response = new_response;
    }
    
    ResponseType responseType = static_cast<ResponseType>(response[1]);
    uint8_t responseId = response[2]; 
    
    
    
    auto handler_it = m_response_handlers.find(responseType);
    if (handler_it != m_response_handlers.end()) {
  
        handler_it->second(response, responseId);
    } else {
        std::cout << "Unknown response type: " << static_cast<int>(responseType) << std::endl;
    }
}

void AltairServer::init_response_handlers() 
{
    // Map each response type to its handler function
    m_response_handlers[TIME_REQUEST] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_time_request(response, responseId);
    };
    
    m_response_handlers[SENSOR_LOG] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_sensor_log(response, responseId);
    };
    
    m_response_handlers[TOTAL_LOGS] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_sensor_log_end(response, responseId);
    };
    
    m_response_handlers[ACK] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_ack(response, responseId);
    };
    
    m_response_handlers[NACK] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_nack(response, responseId);
    };
    
    m_response_handlers[EVENT] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_event(response, responseId);
    };
    
    m_response_handlers[EVENT_LOG] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_event_log(response, responseId);
    };
    
    m_response_handlers[EVENT_LOG_END] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_event_log_end(response, responseId);
    };
    
    m_response_handlers[RESPONSE_CURRENT_TIME] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_response_current_time(response, responseId);
    };
    
    m_response_handlers[BEACON] = [this](std::vector<uint8_t>& response, uint8_t responseId) {
        this->handle_beacon(response, responseId);
    };
}

void AltairServer::handle_time_request(std::vector<uint8_t>&, uint8_t) 
{
    send_current_time();
}

void AltairServer::handle_sensor_log(std::vector<uint8_t>& response, uint8_t responseId) 
{
    //std::cout << "Log id = " << m_total_logs << std::endl;
    SensorData sensor_data;
    m_packet_parser.parse_sensor_data(response, sensor_data);
    m_sensor_data_manager.insertSensorData(sensor_data);
    
    // Check if this log is in response to a client request
    auto it = m_request_clients.find(responseId);
    if (it != m_request_clients.end()) {
        // Format sensor data as string and send to client
        std::string data_str = m_packet_parser.sensor_data_to_string(sensor_data);
        it->second->sendMessage("\nSensor log data:\n" + data_str);
    }
}

void AltairServer::handle_sensor_log_end(std::vector<uint8_t>&, uint8_t responseId) 
{    
    // Check if this is for a tracked request
    auto it = m_request_clients.find(responseId);
    if (it != m_request_clients.end()) {

        it->second->sendMessage("Completed retrieval of sensor logs.\n");
        
        
        // Remove the request from tracking since it's complete
        m_request_clients.erase(it);
    }
}

void AltairServer::handle_ack(std::vector<uint8_t>&, uint8_t responseId) 
{
    auto it = m_request_clients.find(responseId);
    if (it != m_request_clients.end()) {
        it->second->sendMessage("Sucess operation");
        m_request_clients.erase(it);
    }
    
}

void AltairServer::handle_nack(std::vector<uint8_t>&, uint8_t responseId) 
{

    auto it = m_request_clients.find(responseId);
    if (it != m_request_clients.end()) {
        it->second->sendMessage("Request failed. Please try again.");
        m_request_clients.erase(it);
    }
}

void AltairServer::handle_event(std::vector<uint8_t>& response, uint8_t) 
{
    std::cout << "Event" << std::endl;
    EventData event_data;
    m_packet_parser.parse_event_data(response, event_data);
    m_packet_parser.print_event(event_data);
}

void AltairServer::handle_event_log(std::vector<uint8_t>& response, uint8_t responseId) 
{
    EventData event_data;
    m_packet_parser.parse_event_data(response, event_data);
    m_packet_parser.print_event(event_data); 


    auto it = m_request_clients.find(responseId);
    if (it != m_request_clients.end()) {
        std::string data_str = m_packet_parser.event_data_to_string(event_data);
        it->second->sendMessage("\nEvent log data:\n" + data_str);
    }
}

void AltairServer::handle_event_log_end(std::vector<uint8_t>&, uint8_t responseId) 
{
    auto it = m_request_clients.find(responseId);
    if (it != m_request_clients.end()) {
        it->second->sendMessage("\nCompleted retrieval of events logs.\n");
        m_request_clients.erase(it);
    }
}

void AltairServer::handle_response_current_time(std::vector<uint8_t>& response, uint8_t responseId) 
{
    auto it = m_request_clients.find(responseId);
    if (it != m_request_clients.end()) {
        uint32_t current_time = 0;
        uint8_t* uint32_buffer = (uint8_t*)&current_time;
        for (int i = 0; i < 4; ++i) {
            *uint32_buffer++ = response[i + 4];
        }
        std::string local_time = m_packet_parser.format_timestamp(current_time);

        it->second->sendMessage("Current time: " + local_time + "\n");
        m_request_clients.erase(it);
    }
}

void AltairServer::handle_beacon(std::vector<uint8_t>& response, uint8_t) 
{
    m_packet_parser.parse_sensor_data(response, m_latest_data);
    m_packet_parser.print_beacon_data(m_latest_data);
}




void AltairServer::listen()
{
    std::vector<uint8_t> response = std::vector<uint8_t>(0);
    std::vector<uint8_t> char_buffer = std::vector<uint8_t>(0);
 
    while (true) {

        ssize_t bytes_received = m_connection->receive(char_buffer, 1);

        if (bytes_received <= 0) {
             std::cout << "Error" << std::endl;
            continue; // Skip to the next iteration
        }

        if(char_buffer[0] == 0 && response.empty()){
            continue;
        }

        response.push_back(char_buffer[0]);
        
        if(::isalpha(response[0]) || response[0] == '\n'){
            if(char_buffer[0] == '\n')
            {
                std::string debug_message(response.begin(), response.end());
                if(debug_message.length() > 1){
                    std::cout << "Satellite Debug: " << debug_message;
                }

                response.clear();
            }
        }
        else{
            if(char_buffer[0] == END_MARK)
            {
                if((response.size() >= response[0])){
                    handle_response(response);
                    response.clear();
                }
            }
        }
    }
}

void AltairServer::handle_request(const std::string& message, std::shared_ptr<altair::ClientSession> client)
{
    std::cout << "Altair server received message: " << message << std::endl;
    
    // Parse the command from the message
    std::istringstream iss(message);
    std::string command;
    iss >> command;
    
    if (command == "get_sensor_data") {
        // Return the latest sensor data
        std::stringstream response;
        response << "Temperature: " << static_cast<int>(m_latest_data.temp) << "°C, "
                 << "Humidity: " << static_cast<int>(m_latest_data.humid) << "%, "
                 << "Light: " << static_cast<int>(m_latest_data.light) << "%, "
                 << "Voltage: " << m_latest_data.voltage << "V, "
                 << "Mode: ";
        
        switch (m_latest_data.mode) {
            case ERROR_MODE: response << "Error"; break;
            case SAFE_MODE: response << "Safe"; break;
            case OK_MODE: response << "OK"; break;
            default: response << "Unknown"; break;
        }
        
        client->sendMessage(response.str());
    }
    else if (command == "get_recent_sensor_data") {

        if (m_latest_data.timestamp > 0) {
            uint32_t end_time = m_latest_data.timestamp;
            uint32_t start_time = (end_time > 50) ? (end_time - 50) : 0;
            
            get_sensor_in_range(start_time, end_time, client);
            client->sendMessage("Retrieving sensor data from the last minute...");
        } else {
            client->sendMessage("Error: No sensor data available yet. Wait for a beacon.");
        }
    }
    else if (command == "update_light") {
        // Parse the light value
        int light_value;
        iss >> light_value;
        
        if (light_value >= 0 && light_value <= 100) {
            update_light(static_cast<uint8_t>(light_value));
            client->sendMessage("Light updated to " + std::to_string(light_value) + "%");
        } else {
            client->sendMessage("Error: Light value must be between 0 and 100");
        }
    }
    else if (command == "update_min_temp") {
        // Parse the min temp value
        int min_temp;
        iss >> min_temp;
        
        if (iss.fail()) {
            client->sendMessage("Error: Invalid temperature value");
        } else {
            update_min_temp(static_cast<uint8_t>(min_temp));
            client->sendMessage("Minimum temperature updated to " + std::to_string(min_temp) + "°C");
        }
    }
    else if (command == "update_max_temp") {
        // Parse the max temp value
        int max_temp;
        iss >> max_temp;
        
        if (iss.fail()) {
            client->sendMessage("Error: Invalid temperature value");
        } else {
            update_max_temp(static_cast<uint8_t>(max_temp));
            client->sendMessage("Maximum temperature updated to " + std::to_string(max_temp) + "°C");
        }
    }
    else if (command == "update_humidity") {
        // Parse the humidity value
        int humidity;
        iss >> humidity;
        
        if (iss.fail()) {
            client->sendMessage("Error: Invalid humidity value");
        } else if (humidity >= 0 && humidity <= 100) {
            update_humidity(static_cast<uint8_t>(humidity));
            client->sendMessage("Humidity updated to " + std::to_string(humidity) + "%");
        } else {
            client->sendMessage("Error: Humidity value must be between 0 and 100");
        }
    }
    else if (command == "update_voltage") {
        // Parse the voltage value
        float voltage;
        iss >> voltage;
        
        if (iss.fail()) {
            client->sendMessage("Error: Invalid voltage value");
        } else {
            if(voltage > 3.3f || voltage < 0.1f){
                client->sendMessage("Error: Voltage value must be between 0.1 and 3.3");
                return;
            }
            else{
                update_voltage(voltage);
                client->sendMessage("Voltage updated to " + std::to_string(voltage) + "V");
            }
        }
    }
    else if (command == "get_sensor_logs") {
        // Parse start and end timestamps
        uint32_t start, end;
        iss >> start >> end;
        
        if (iss.fail()) {
            client->sendMessage("Error: Invalid timestamp values. Format: get_logs <start_timestamp> <end_timestamp>");
        } else {
            get_sensor_in_range(start, end, client);
            client->sendMessage("Requested logs between " + std::to_string(start) + " and " + std::to_string(end) + ". Processing...");
        }
    }
    else if (command == "get_events_logs") {
        // Parse start and end timestamps
        uint32_t start, end;
        iss >> start >> end;
        
        if (iss.fail()) {
            client->sendMessage("Error: Invalid timestamp values. Format: get_events_logs <start_timestamp> <end_timestamp>");
        } else {
            get_event_in_range(start, end, client);
            client->sendMessage("Requested logs between " + std::to_string(start) + " and " + std::to_string(end) + ". Processing...");
        }
    }
    else if (command == "get_current_time") {
        get_current_time(client);

    }
        else if (command == "set_time") {
        // Parse the new time value
        uint32_t new_time;
        iss >> new_time;
        
        if (iss.fail()) {
            client->sendMessage("Error: Invalid time value. Format: set_time <unix_timestamp>");
        } else {
            // Simple validation - don't allow setting time before the latest data timestamp
            if (m_latest_data.timestamp > 0 && new_time < m_latest_data.timestamp) {
                client->sendMessage("Error: Cannot set time before the latest sensor data timestamp (" 
                                  + std::to_string(m_latest_data.timestamp) + ")");
            } else {
                // Store the new time to be sent when satellite requests time
                send_custom_time(new_time);
                //set new_time string
                std::string new_time_str = m_packet_parser.format_timestamp(new_time);

                client->sendMessage("\nSet custom time to:" + new_time_str + "\n");
            }
        }
    }
    else if (command == "help") {
        // Send the available commands in a more organized and attractive format
        std::string help_message =
            "🛰️ === ALTAIR SATELLITE COMMAND CENTER === 🛰️\n\n"
            "📊 SENSOR DATA COMMANDS:\n"
            "  • get_sensor_data         - Get the latest sensor readings\n"
            "  • get_recent_sensor_data  - Get sensor data from the last minute\n\n"
            
            "⏰ TIME MANAGEMENT:\n"
            "  • get_current_time        - Get the current time from the satellite\n"
            "  • set_time <timestamp>    - Set custom time for the satellite\n\n"
            
            "🔧 SATELLITE CONFIGURATION:\n"
            "  • update_light <value>    - Set light level (0-100)\n"
            "  • update_min_temp <value> - Set minimum temperature\n"
            "  • update_max_temp <value> - Set maximum temperature\n"
            "  • update_humidity <value> - Set humidity level (0-100)\n"
            "  • update_voltage <value>  - Set voltage level (0.1-3.3V)\n\n"
            
            "📝 LOG RETRIEVAL:\n"
            "  • get_sensor_logs <start> <end> - Request sensor logs between timestamps (MAX 10)\n"
            "  • get_events_logs <start> <end> - Request events logs between timestamps (MAX 10)\n\n"
            
            "ℹ️ HELP:\n"
            "  • help                    - Show this help message\n\n";
            

        
        client->sendMessage(help_message);
    }
        
    else {
        // Unknown command
        client->sendMessage("Unknown command: " + command + ". Type 'help' for available commands.");
    }
}



void AltairServer::get_current_time(std::shared_ptr<altair::ClientSession> client)
{
    MessagePacket packet = m_packet_parser.create_message_packet(ResponseType::REQUEST_CURRENT_TIME, m_id_generator.generateID());
    packet.data_len += sizeof(uint32_t);

    m_request_clients[packet.m_respnse_id] = client;
    
    // std::cout << "Storing request ID " << static_cast<int>(packet.m_respnse_id) 
    //           << " for client " << client->getClientId() << std::endl;

    send_packet_to_altair(packet);
}

void AltairServer::get_event_in_range(uint32_t start, uint32_t end, std::shared_ptr<altair::ClientSession> client)
{

    MessagePacket packet = m_packet_parser.create_message_packet(ResponseType::REQUEST_EVENT_LOG, m_id_generator.generateID());
    packet.data_len += (sizeof(uint32_t) * 2);

    // Store the request ID and client in the hashmap
    m_request_clients[packet.m_respnse_id] = client;
    
    // std::cout << "Storing request ID " << static_cast<int>(packet.m_respnse_id) 
    //           << " for client " << client->getClientId() << std::endl;

    uint8_t* uint32_buffer = (uint8_t*)&start;

    for (int i = 0; i < 4; ++i) {
        packet.buffer[i] = *uint32_buffer++;
    }

    uint32_buffer = (uint8_t*)&end;

    for (int i = 0; i < 4; ++i) {
        packet.buffer[i + 4] = *uint32_buffer++;
    }

    send_packet_to_altair(packet);
}

void AltairServer::get_sensor_in_range(uint32_t start, uint32_t end, std::shared_ptr<altair::ClientSession> client)
{

    MessagePacket packet = m_packet_parser.create_message_packet(ResponseType::REQUEST_SENSOR_LOGS, m_id_generator.generateID());
    packet.data_len += (sizeof(uint32_t) * 2);

    // Store the request ID and client in the hashmap
    m_request_clients[packet.m_respnse_id] = client;
    
    uint8_t* uint32_buffer = (uint8_t*)&start;

    for (int i = 0; i < 4; ++i) {
        packet.buffer[i] = *uint32_buffer++;
    }

    uint32_buffer = (uint8_t*)&end;

    for (int i = 0; i < 4; ++i) {
        packet.buffer[i + 4] = *uint32_buffer++;
    }

    send_packet_to_altair(packet);
}

void AltairServer::send_custom_time(uint32_t custom_time)
{
    send_value<uint32_t>(ResponseType::TIME_SEND, custom_time);
}

void AltairServer::send_current_time()
{
    auto now = std::chrono::system_clock::now();
    uint32_t epoch_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    send_value<uint32_t>(ResponseType::TIME_SEND, epoch_time);
    
    std::cout << "Sending time " << epoch_time << std::endl;
}

void AltairServer::update_max_temp(uint8_t max_temp)
{
    send_value<uint8_t>(ResponseType::UPDATE_MAX_TEMP, max_temp);
}

void AltairServer::update_min_temp(uint8_t min_temp)
{
    send_value<uint8_t>(ResponseType::UPDATE_MIN_TEMP, min_temp);
}

void AltairServer::update_humidity(uint8_t humidity)
{
    send_value<uint8_t>(ResponseType::UPDATE_HUMIDITY, humidity);
}

void AltairServer::update_light(uint8_t light)
{
    send_value<uint8_t>(ResponseType::UPDATE_LIGHT, light);
}

void AltairServer::update_voltage(float voltage)
{
    send_value<float>(ResponseType::UPDATE_VOLTAGE, voltage);
}

template<typename T>
void AltairServer::send_value(ResponseType type, const T& value) 
{
    
    MessagePacket packet =  m_packet_parser.create_message_packet(type, m_id_generator.generateID());
    packet.data_len += sizeof(T);
    
    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(&value);
    
    for (size_t i = 0; i < sizeof(T); ++i) {
        packet.buffer[i] = data_ptr[i];
    }
    
    send_packet_to_altair(packet);
}

void AltairServer::send_packet_to_altair(MessagePacket& message_packet)
{
    std::vector<uint8_t> message_buffer;

    message_buffer.push_back(message_packet.data_len);
    message_buffer.push_back(message_packet.packetType);

    // id 0xFF are reserved for satellite callbacks (Beacon, Event)
    // if the id is 0xFF, generate a new one
    if(message_packet.m_respnse_id == 0xFF){
        message_packet.m_respnse_id = m_id_generator.generateID();
    }
    message_buffer.push_back(message_packet.m_respnse_id);
    message_buffer.push_back(message_packet.checksum);

    for (int i = 0; i < message_packet.data_len - PACKET_HEADER_SIZE; ++i) {
        message_buffer.push_back(message_packet.buffer[i]);
    }

    message_buffer.push_back(message_packet.end_mark);
    
    m_connection->send(message_buffer);
}

} // namespace altair
