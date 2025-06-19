#ifndef SERVER_DATA_MANAGER_HPP
#define SERVER_DATA_MANAGER_HPP

#include "packet_parser.hpp"
#include <vector>
#include <mutex>
#include <optional>

namespace altair {

/**
 * @class ServerDataManager
 * @brief Manages a collection of SensorData objects sorted by timestamp
 * 
 * This class provides efficient storage and retrieval of SensorData objects
 * with binary search insertion to maintain sorted order by timestamp.
 */
class ServerDataManager {
public:
    /**
     * @brief Default constructor
     */
    ServerDataManager();

    /**
     * @brief Insert SensorData into the collection using binary search
     * @param data The SensorData to insert
     * @return True if insertion was successful, false otherwise
     */
    bool insertSensorData(const SensorData& data);

    /**
     * @brief Get SensorData by timestamp
     * @param timestamp The timestamp to look for
     * @return Optional SensorData - has value if found, empty if not found
     */
    std::optional<SensorData> getSensorDataByTimestamp(uint32_t timestamp) const;

    /**
     * @brief Get all SensorData in a given time range
     * @param start_time Start of the time range
     * @param end_time End of the time range (inclusive)
     * @return Vector of SensorData objects within the range
     */
    std::optional<std::vector<SensorData>> getSensorDataInRange(uint32_t start_time, uint32_t end_time) const;

    /**
     * @brief Get the most recent SensorData
     * @return Optional SensorData - has value if collection is not empty
     */
    std::optional<SensorData> getMostRecentData() const;

    /**
     * @brief Get all SensorData
     * @return Copy of the entire collection
     */
    std::vector<SensorData> getAllSensorData() const;

    /**
     * @brief Get the number of stored SensorData objects
     * @return Size of the collection
     */
    size_t size() const;

    /**
     * @brief Clear all stored data
     */
    void clear();

private:
    std::vector<SensorData> m_sensor_data; // Collection of sensor data sorted by timestamp
    mutable std::mutex m_mutex; // Mutex for thread safety
};

} // namespace altair

#endif // SERVER_DATA_MANAGER_HPP
