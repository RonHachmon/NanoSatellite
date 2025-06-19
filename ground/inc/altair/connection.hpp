#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <vector> 
#include <cstdint>
#include <sys/types.h> // for ssize_t

namespace altair {

/**
 * @class Connection
 * @brief Abstract base class for communication connections
 *
 * This class defines the interface for different types of connections
 * used in the Altair system (such as serial, network, etc). It provides
 * a common interface for sending and receiving data, allowing the system
 * to work with different connection types interchangeably.
 *
 * Derived classes must implement the send() and receive() methods to handle
 * the specific communication protocol they represent.
 */
class Connection {
public:
    /**
     * @brief Virtual destructor
     *
     * Virtual destructor ensures proper cleanup of derived classes
     * when deleted through a base class pointer.
     */
    virtual ~Connection() = default;

    /**
     * @brief Send data through the connection
     * 
     * @param message Vector of bytes to send
     * @return ssize_t Number of bytes sent, or negative value on error
     */
    virtual ssize_t send(const std::vector<uint8_t>& message) = 0;

    /**
     * @brief Receive data from the connection
     * 
     * @param message Reference to a vector where received data will be stored
     * @param size Maximum number of bytes to receive
     * @return ssize_t Number of bytes received, or negative value on error
     */
    virtual ssize_t receive(std::vector<uint8_t>& message, uint8_t size) = 0;

protected:
    /**
     * @brief Default constructor
     *
     * Protected to prevent direct instantiation of the abstract base class.
     * Only derived classes can be instantiated.
     */
    Connection() = default;
    
    /**
     * @brief Copy constructor
     *
     * Protected to allow derived classes to implement copy operations if needed.
     */
    Connection(const Connection&) = default;
    
    /**
     * @brief Assignment operator
     *
     * Protected to allow derived classes to implement assignment if needed.
     * 
     * @return Reference to this Connection instance
     */
    Connection& operator=(const Connection&) = default; 
};

} // namespace altair

#endif // CONNECTION_HPP
