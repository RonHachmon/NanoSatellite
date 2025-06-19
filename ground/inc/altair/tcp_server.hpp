#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <boost/asio.hpp>

namespace altair {

// Forward declarations
class ClientSession;

/**
 * @class TcpServer
 * @brief Main server class responsible for accepting client connections
 */
class TcpServer {
public:
    /**
     * @brief Constructor for TcpServer
     * @param port The port number to listen on
     * @param maxConnections Maximum number of concurrent connections allowed
     */
    explicit TcpServer(unsigned short port = 4444, size_t maxConnections = 100);
    
    /**
     * @brief Destructor, ensures clean shutdown
     */
    ~TcpServer();
    
    /**
     * @brief Starts the server
     * @return true if server started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stops the server and all client connections
     */
    void stop();
    
    /**
     * @brief Registers a callback for handling received messages
     * @param handler Function to be called when a message is received
     */
    void setMessageHandler(std::function<void(const std::string&, std::shared_ptr<ClientSession>)> handler);
    
    /**
     * @brief Broadcasts a message to all connected clients
     * @param message The message to broadcast
     */
    void broadcastMessage(const std::string& message);
    
    /**
     * @brief Gets the current number of connected clients
     * @return The number of active connections
     */
    size_t getClientCount() const;
    
private:
    // ASIO service and acceptor
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    // Server properties
    unsigned short port_;
    size_t maxConnections_;
    std::atomic<bool> running_;
    
    // Client management
    std::unordered_map<size_t, std::shared_ptr<ClientSession>> clients_;
    mutable std::mutex clientsMutex_;
    size_t nextClientId_;
    
    // Message handling
    std::function<void(const std::string&, std::shared_ptr<ClientSession>)> messageHandler_;
    
    // Worker thread for running the io_context
    std::unique_ptr<std::thread> ioThread_;
    
    /**
     * @brief Starts accepting new connections
     */
    void startAccept();
    
    /**
     * @brief Callback for when a new connection is accepted
     * @param client The newly created client session
     * @param error Any error that occurred during acceptance
     */
    void handleAccept(std::shared_ptr<ClientSession> client, const boost::system::error_code& error);
    
    /**
     * @brief Adds a new client to the clients collection
     * @param client The client to add
     * @return The assigned client ID
     */
    size_t addClient(std::shared_ptr<ClientSession> client);
    
    /**
     * @brief Removes a client from the clients collection
     * @param clientId The ID of the client to remove
     */
    void removeClient(size_t clientId);
    
    /**
     * @brief Friend declaration to allow ClientSession to access removeClient
     */
    friend class ClientSession;
};

/**
 * @class ClientSession
 * @brief Represents a connected client and manages communication with it
 */
class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    /**
     * @brief Constructor for ClientSession
     * @param io_context ASIO io_context to use for this session
     * @param server Pointer to the parent server
     */
    ClientSession(boost::asio::io_context& io_context, TcpServer* server);
    
    /**
     * @brief Destructor
     */
    ~ClientSession();
    
    /**
     * @brief Gets the socket for this session
     * @return Reference to the socket
     */
    boost::asio::ip::tcp::socket& socket();
    
    /**
     * @brief Starts the client session
     * @param clientId The ID assigned to this client
     */
    void start(size_t clientId);
    
    /**
     * @brief Stops the client session
     */
    void stop();
    
    /**
     * @brief Sends a message to the client
     * @param message The message to send
     */
    void sendMessage(const std::string& message);
    
    /**
     * @brief Gets the client's ID
     * @return The client ID
     */
    size_t getClientId() const;
    
    /**
     * @brief Gets the client's remote address
     * @return String representation of the client's address
     */
    std::string getRemoteAddress() const;
    
private:
    // ASIO socket
    boost::asio::ip::tcp::socket socket_;
    
    // Parent server
    TcpServer* server_;
    
    // Client properties
    size_t clientId_;
    std::string remoteAddress_;
    
    // Buffer for receiving data
    enum { MAX_BUFFER_SIZE = 8192 };
    char dataBuffer_[MAX_BUFFER_SIZE];
    
    // State management
    std::atomic<bool> active_;
    
    /**
     * @brief Starts asynchronous read operation
     */
    void startRead();
    
    /**
     * @brief Callback for when data is read from the socket
     * @param error Any error that occurred during reading
     * @param bytesTransferred Number of bytes read
     */
    void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
    
    /**
     * @brief Callback for when data is written to the socket
     * @param error Any error that occurred during writing
     * @param bytesTransferred Number of bytes written
     */
    void handleWrite(const boost::system::error_code& error, size_t bytesTransferred);
};

} // namespace altair

#endif // TCP_SERVER_HPP
