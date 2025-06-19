#include "packet_parser.hpp"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <string>

namespace altair 
{

ResponseType PacketParser::parse_response_type(const std::vector<uint8_t>& response) const 
{
    if (response.size() < 2) {
        return ResponseType::UNKNOWN;
    }
    return static_cast<ResponseType>(response[1]);
}

std::string PacketParser::sensor_data_to_string(const SensorData& data) const
{
    std::stringstream ss;
    ss << "Temperature: " << static_cast<int>(data.temp) << "°C\n"
       << "Humidity: " << static_cast<int>(data.humid) << "%\n"
       << "Light: " << static_cast<int>(data.light) << "%\n"
       << "Mode: " << get_mode_string(static_cast<AltairModes>(data.mode)) << "\n"
       << "Voltage: " << std::fixed << std::setprecision(2) << data.voltage << "V\n"
       << "Timestamp: " << data.timestamp << "\n"
       << "Local Time: "  << format_timestamp(data.timestamp)  << std::endl;
    return ss.str();
}

std::string PacketParser::event_data_to_string(const EventData& data) const
{
    std::stringstream ss;
    ss << "Event: " << get_event_string(data.event) << "\n"
       << "Timestamp: " << data.timestamp;
    return ss.str();
}

void PacketParser::parse_sensor_data(const std::vector<uint8_t>& response, SensorData& data) const 
{
    size_t len = response.size();
    size_t idx = 4;

    if (len == 0) {
        return;
    }

    data.temp = response[idx++];
    data.humid = response[idx++];
    data.light = response[idx++];
    data.mode = static_cast<AltairModes>(response[idx++]);
    
    
    // Read float voltage (4 bytes)
    
    uint8_t* float_buffer = (uint8_t*)&data.voltage;
    for (int i = 0; i < 4; ++i) {
        *float_buffer++ = response[idx + i];
    }


    idx += 4;
    
    // Read uint32_t timestamp (4 bytes)
    if (len > idx + 3) {
        uint8_t* uint32_t_buffer = (uint8_t*)&data.timestamp;
        for (int i = 0; i < 4; ++i) {
            *uint32_t_buffer++ = response[idx + i];
        }
    }  
}

void PacketParser::parse_event_data(const std::vector<uint8_t>& response, EventData& event_data) const 
{
    size_t len = response.size();
    size_t idx = 4;

    event_data.event = static_cast<AltairEvent>(response[idx++]);
    

    // Read uint32_t timestamp (4 bytes)
    if (idx + 3 < len) {
        uint8_t* uint32_buffer = (uint8_t*)&event_data.timestamp;
        for (int i = 0; i < 4; ++i) {
            *uint32_buffer++ = response[idx + i];
        }
    }

}

void PacketParser::print_beacon_data(const SensorData& data) const
{

    std::cout << "Beacon Data:" << std::endl;

    std::cout << "Mode: " << get_mode_string(data.mode) << std::endl;
    std::cout << "Timestamp: " << data.timestamp << std::endl;
      std::cout << "Local Time: "  << format_timestamp(data.timestamp)  << std::endl;
    std::cout << "-----------------" << std::endl;
}

void PacketParser::print_sensor_data(const SensorData& data) const
{

    std::cout << "Sensor Data:" << std::endl;
    std::cout << "Temp: °" << (int)data.temp << "C" << std::endl;
    std::cout << "Humidity: " << (int)data.humid << "%" << std::endl;
    std::cout << "Light: " << (int)data.light << "%" << std::endl;
    std::cout << "Mode: " << get_mode_string(data.mode) << std::endl;
    std::cout << "Voltage: " << data.voltage << "V" << std::endl;
    std::cout << "Timestamp: " << data.timestamp << std::endl;
    std::cout << "Local Time: "  << format_timestamp(data.timestamp)  << std::endl;
    std::cout << "-----------------" << std::endl;
}

void PacketParser::print_event(const EventData& data) const 
{
    std::cout << "Event: " << get_event_string(data.event) << std::endl;
    std::cout << "Timestamp: " << data.timestamp << std::endl;
    std::cout << "Local Time: "  << format_timestamp(data.timestamp)  << std::endl;
    std::cout << "-----------------" << std::endl;
}

std::string PacketParser::format_timestamp(time_t timestamp) const
{
    struct tm* timeinfo = localtime(&timestamp);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z", timeinfo);
    return std::string(buffer);
}

std::string PacketParser::get_mode_string(AltairModes mode) const 
{
    switch (mode) {
        case ERROR_MODE:
            return "Error";
        case SAFE_MODE:
            return "Safe";
        case OK_MODE:
            return "OK";
        default:
            return "Unknown";
    }
}


std::string PacketParser::get_event_string(AltairEvent event) const 
{
    switch (event) {
        case OK_TO_ERROR:
            return "OK to Error";
        case ERROR_TO_OK:
            return "Error to OK";
        case WD_RESET:
            return "Watchdog Reset";
        case INIT:
            return "Initialization";
        case OK_TO_SAFE:
            return "OK to Safe";
        case SAFE_TO_ERROR:
            return "Safe to Error";
        case SAFE_TO_OK:
            return "Safe to OK";
        case ERROR_TO_SAFE:
            return "Error to safe";
            
        default:
            return "Error to 0K";;
    }
}

MessagePacket PacketParser::create_message_packet(ResponseType type, uint8_t response_id) 
{
    MessagePacket packet;
    packet.packetType = type;
    packet.data_len = PACKET_HEADER_SIZE;
    packet.m_respnse_id = response_id;
    packet.checksum = 0x00;
    packet.end_mark = END_MARK;
    return packet;
}

bool PacketParser::is_valid_response(const std::vector<uint8_t>& response) const 
{
    // Basic validation checks
    if (response.empty()) {
        return false;
    }
    
    // Check if the response has at least the header size
    if (response.size() < PACKET_HEADER_SIZE) {
        return false;
    }
    
    // Check if the last byte is the end mark
    if (response.back() != END_MARK) {
        return false;
    }
    
    // Check if the data length matches
    if (response.size() != response[0]) {
        return false;
    }
    
    return true;
}



} // namespace altair
