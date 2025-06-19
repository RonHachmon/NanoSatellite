#ifndef ID_GENERATOR_HPP
#define ID_GENERATOR_HPP

#include <cstdint>
#include <mutex>

namespace altair {

/**
 * @class IDGenerator
 * @brief Thread-safe singleton class for generating unique IDs
 *
 * This class generates sequential 8-bit IDs for tracking request/response pairs
 * in the Altair satellite communication system. It implements the Singleton
 * design pattern to ensure that only one instance exists throughout the application,
 * preventing ID collisions between different components.
 */
class IDGenerator {
public:
    /**
     * @brief Get the singleton instance of IDGenerator
     * 
     * This method ensures that only one instance of IDGenerator exists.
     * The instance is created on first call and persists for the lifetime
     * of the application.
     * 
     * @return Reference to the singleton IDGenerator instance
     */
    static IDGenerator& getInstance();

    /**
     * @brief Generate the next unique ID
     * 
     * This method is thread-safe and will generate sequential IDs starting from 0.
     * The IDs are 8-bit values that wrap around after reaching 255, returning to 0.
     * 
     * @return The next unique ID
     */
    uint8_t generateID();

private:
    /**
     * @brief Private constructor to prevent direct instantiation
     * 
     * Initializes the current_id to 0. This constructor is private to enforce
     * the Singleton pattern, preventing multiple instances of the class.
     */
    IDGenerator();

    /**
     * @brief Deleted copy constructor to prevent copying
     * 
     * This prevents copying of the singleton instance, which could lead to
     * multiple instances and thus possible ID collisions.
     */
    IDGenerator(const IDGenerator&) = delete;
    
    /**
     * @brief Deleted assignment operator to prevent copying
     * 
     * This prevents assignment of the singleton instance, which could lead to
     * multiple instances and thus possible ID collisions.
     */
    IDGenerator& operator=(const IDGenerator&) = delete;

    uint8_t current_id;  ///< Current ID value, incremented with each generation
    std::mutex mtx;      ///< Mutex for thread-safe ID generation
};

}  // namespace altair

#endif // ID_GENERATOR_HPP