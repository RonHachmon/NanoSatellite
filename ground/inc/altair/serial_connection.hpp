#ifndef SERIAL_CONNECTION_HPP
#define SERIAL_CONNECTION_HPP

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "connection.hpp"

namespace altair {

/**
 * @class SerialConnection
 * @brief Implements the Connection interface for serial port communication
 *
 * This class provides functionality for communicating with the Altair satellite
 * over a serial port connection. It handles the low-level details of serial port
 * configuration, data transmission, and reception.
 *
 * The class supports move semantics but not copy semantics, as copying a 
 * serial port connection could lead to resource conflicts.
 */
class SerialConnection : public Connection {
public:
    /**
     * @brief Constructs a SerialConnection with the specified port
     *
     * Opens the serial port and configures it with appropriate settings
     * for satellite communication (baud rate, parity, etc).
     *
     * @param port Path to the serial port device (e.g., "/dev/ttyUSB0")
     */
    explicit SerialConnection(const std::string& port);
    
    /**
     * @brief Copy constructor (deleted)
     *
     * Copying a serial connection is not allowed as it would lead to 
     * multiple objects managing the same file descriptor.
     */
    SerialConnection(const SerialConnection&) = delete;
    
    /**
     * @brief Copy assignment operator (deleted)
     *
     * Copying a serial connection is not allowed as it would lead to 
     * multiple objects managing the same file descriptor.
     */
    SerialConnection& operator=(const SerialConnection&) = delete;
    
    /**
     * @brief Move constructor
     *
     * Allows transferring ownership of the serial port from one 
     * SerialConnection object to another.
     *
     * @param other The SerialConnection to move from
     */
    SerialConnection(SerialConnection&&) noexcept;
    
    /**
     * @brief Move assignment operator
     *
     * Allows transferring ownership of the serial port from one 
     * SerialConnection object to another.
     *
     * @param other The SerialConnection to move from
     * @return Reference to this SerialConnection instance
     */
    SerialConnection& operator=(SerialConnection&&) noexcept;
    
    /**
     * @brief Destructor
     *
     * Closes the serial port file descriptor if it's open.
     */
    ~SerialConnection();
    
    /**
     * @brief Sends a message over the serial connection
     *
     * Implements the send() method from the Connection interface.
     *
     * @param message Vector of bytes to send
     * @return Number of bytes sent, or negative value on error
     */
    ssize_t send(const std::vector<uint8_t>& message) override;
    
    /**
     * @brief Receives data from the serial connection
     *
     * Implements the receive() method from the Connection interface.
     *
     * @param message Reference to a vector where received data will be stored
     * @param size Maximum number of bytes to receive
     * @return Number of bytes received, or negative value on error
     */
    ssize_t receive(std::vector<uint8_t>& message, uint8_t size) override;
    
    /**
     * @brief Checks if the serial connection is valid
     *
     * @return true if the serial port is open and ready for communication,
     *         false otherwise
     */
    bool is_valid();
    
    /**
     * @brief Reads data from the serial port until an end marker or newline
     *
     * This method is more specialized than receive() and attempts to read
     * a complete message based on content-specific termination conditions.
     *
     * @param out_data Reference to a vector where the read data will be stored
     * @return Number of bytes read, 0 if no data available, or negative on error
     */
    int read_data(std::vector<uint8_t>& out_data);
    
    /**
     * @brief Writes a string to the serial port
     *
     * @param data The string to write
     * @return Number of bytes written, or negative value on error
     */
    int write_data(const std::string& data);
    
    /**
     * @brief Writes a single character to the serial port
     *
     * @param ch The character to write
     * @return Number of bytes written, or negative value on error
     */
    int write_data(char ch);
    
private:
    int m_serial_port;  ///< File descriptor for the serial port
};

} // namespace altair

#endif // SERIAL_CONNECTION_HPP