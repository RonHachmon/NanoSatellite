#include "server_data_manager.hpp"
#include <algorithm>

namespace altair {

ServerDataManager::ServerDataManager() 
{
    // Initialize an empty container
    m_sensor_data.reserve(100); // Pre-allocate space for 100 entries as a starting point
}

bool ServerDataManager::insertSensorData(const SensorData& data) 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Find the position to insert using binary search
    auto pos = std::lower_bound(m_sensor_data.begin(), m_sensor_data.end(), data,
        [](const SensorData& a, const SensorData& b) {
            return a.timestamp < b.timestamp;
        });
    
    // Check if this is a duplicate (same timestamp)
    if (pos != m_sensor_data.end() && pos->timestamp == data.timestamp) {
        return true;
    }
    
    // Insert at the correct position
    m_sensor_data.insert(pos, data);
    return true;
}

std::optional<SensorData> ServerDataManager::getSensorDataByTimestamp(uint32_t timestamp) const 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    

    auto pos = std::lower_bound(m_sensor_data.begin(), m_sensor_data.end(), timestamp,
        [](const SensorData& a, uint32_t timestamp) {
            return a.timestamp < timestamp;
        });
    
    if (pos != m_sensor_data.end() && pos->timestamp == timestamp) {
        return *pos;
    }
    
    return std::nullopt;
}

std::optional<std::vector<SensorData>> ServerDataManager::getSensorDataInRange(uint32_t start_time, uint32_t end_time) const 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Check if the data is empty
    if (m_sensor_data.empty()) {
        return std::nullopt;
    }
    
    // Check if start_time is beyond the latest data timestamp
    if (start_time > m_sensor_data.back().timestamp) {
        return std::nullopt;
    }
    
    std::vector<SensorData> result;
    
    // Find the lower bound of the range
    auto lower = std::lower_bound(m_sensor_data.begin(), m_sensor_data.end(), start_time,
        [](const SensorData& a, uint32_t timestamp) {
            return a.timestamp < timestamp;
        });
    
    // Find the upper bound of the range
    auto upper = std::upper_bound(m_sensor_data.begin(), m_sensor_data.end(), end_time,
        [](uint32_t timestamp, const SensorData& a) {
            return timestamp < a.timestamp;
        });
    
    // Copy data in range to result vector
    result.reserve(std::distance(lower, upper));
    for (auto it = lower; it != upper; ++it) {
        result.push_back(*it);
    }
    
    
    return result;
}

std::optional<SensorData> ServerDataManager::getMostRecentData() const 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_sensor_data.empty()) {
        return std::nullopt;
    }
    
    return m_sensor_data.back();
}

std::vector<SensorData> ServerDataManager::getAllSensorData() const 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sensor_data;
}

size_t ServerDataManager::size() const 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sensor_data.size();
}

void ServerDataManager::clear() 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sensor_data.clear();
}

} // namespace altair
