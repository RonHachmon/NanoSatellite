#include "serial_connection.hpp"

namespace altair {

SerialConnection::SerialConnection(const std::string& port):
m_serial_port{-1}
{
    m_serial_port = open(port.c_str(), O_RDWR | O_NOCTTY);
    if (m_serial_port == -1) {
        std::cout << "Error opening serial port: " << strerror(errno) << std::endl;
    } else {
        //std::cout << "Sucess: " << m_serial_port << std::endl;
        // Configure serial settings
        struct termios tty;
        if (tcgetattr(m_serial_port, &tty) != 0) {
            std::cout << "Error getting terminal attributes: " << strerror(errno) << std::endl;
            close(m_serial_port);
            m_serial_port = -1;
        }
        else {
            cfsetispeed(&tty, B115200);
            cfsetospeed(&tty, B115200);

            tty.c_cflag &= ~PARENB;  // No parity
            tty.c_cflag &= ~CSTOPB;  // One stop bit
            tty.c_cflag &= ~CSIZE;
            tty.c_cflag |= CS8;      // 8 bits per byte

            tty.c_cflag &= ~CRTSCTS;  // No hardware flow control
            tty.c_cflag |= CREAD | CLOCAL;  // Enable receiver

            tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw mode
            tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // No software flow control
            tty.c_oflag &= ~OPOST;  // Raw output
            tty.c_cc[VTIME] = 0;  // No timeout

            int flags = fcntl(m_serial_port, F_GETFL, 0);
            if (flags & O_NONBLOCK) {
                fcntl(m_serial_port, F_SETFL, flags & ~O_NONBLOCK); // Set to blocking mode
            }

            if (tcsetattr(m_serial_port, TCSANOW, &tty) != 0) {
                std::cout << "Error setting terminal attributes: " << strerror(errno) << std::endl;
                close(m_serial_port);
                m_serial_port = -1;
            }
        }
    }
}


SerialConnection::~SerialConnection() {
    if (m_serial_port != -1) {
        close(m_serial_port);
    }
    m_serial_port = -1;
}



SerialConnection::SerialConnection(SerialConnection&& other) noexcept
: m_serial_port(other.m_serial_port)
{
    other.m_serial_port = -1;  
}

SerialConnection& SerialConnection::operator=(SerialConnection&& other) noexcept
{
    if (this != &other) {
        if (m_serial_port != -1) {
            close(m_serial_port);
        }
        m_serial_port = other.m_serial_port;
        other.m_serial_port = -1;  
    }
    return *this;
}

bool SerialConnection::is_valid()
{
    return m_serial_port != -1;
}

ssize_t SerialConnection:: receive(std::vector<uint8_t>& message, uint8_t size)
{
    if (m_serial_port == -1) {
        return -1;  // Return 0 if the serial port is not valid
    }

    message.clear();  
    char value;
    message.resize(size);

    ssize_t bytes_read = 0;
    do {
        bytes_read  = ::read(m_serial_port, &value, size);
    }while (bytes_read == -1 && errno == EINTR); 

    if (bytes_read > 0) {
        message.resize(bytes_read);  
        message[0] = value;  // Store the read byte in the first position of the vector
        if(message[0] == 0){
            //std::cout << "Received 0x00 ;-;" << std::endl;
        }
    }
    else
    {
        //print errno
        //std::cout << "Error reading from serial port: " << strerror(errno) << std::endl;
    }
    return bytes_read;  
}
ssize_t SerialConnection:: send(const std::vector<uint8_t>& message)
{
    if (m_serial_port == -1) {
        return false;
    }

    ssize_t bytes_written = 0;
    do {
        bytes_written = ::write(m_serial_port, message.data(), message.size());  
    } while (bytes_written == -1 && errno == EINTR);  

    return bytes_written ;
}





// Read data from the serial port
int SerialConnection:: read_data(std::vector<uint8_t>& out_data) 
{
    if (m_serial_port == -1) {
        return 0;  // Return 0 if the serial port is not valid
    }

    uint8_t byte;
    out_data.clear();  // Clear out_data before reading new data.

    while (true) {
        int bytes_read = read(m_serial_port, &byte, 1);  // Read one byte at a time

        if (bytes_read > 0) {
            out_data.push_back(byte);  // Add byte to out_data

            // Check if the first byte is 12, and if so, stop reading when 0x55 is encountered
            if (out_data[0] == 12 || out_data[0] == 0) {
                if (byte == 0x55) {
                    return out_data.size();  // Return number of bytes read until 0x55
                }
            } else {
                // If first byte is not 12, stop reading when '\n' is encountered
                if (byte == '\n') {
                    return out_data.size();  // Return number of bytes read until '\n'
                }
            }
        } else if (bytes_read == 0) {
            // No data available, return 0 to indicate nothing was read
            return 0;
        } else {
            // Handle error, e.g., return -1 or log it
            return -1;
        }

    }
            
    if(out_data.size() >100){
        return -1;
    }
}

// Write data to serial port
int SerialConnection::write_data(const std::string& data) {
    if (m_serial_port == -1) {
        return 0;
    }

    int bytes_written = write(m_serial_port, data.c_str(), data.length());
    return bytes_written;
}

int SerialConnection::write_data(char ch) {
    if (m_serial_port == -1) {
        return 0;
    }

    int bytes_written = write(m_serial_port, &ch, 1);  // Write one character at a time
    if (bytes_written > 0) {
        return bytes_written;  // Return number of bytes written (1 if successful)
    } else {
        return -1;  // Return -1 if writing failed
    }
}

} // namespace altair
