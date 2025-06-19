#include "tcp_server.hpp"
#include <iostream>

namespace altair {

// TcpServer implementation
TcpServer::TcpServer(unsigned short port, size_t maxConnections):
port_(port),
maxConnections_(maxConnections),
running_(false),
nextClientId_(1),
acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    messageHandler_ = [](const std::string& message, std::shared_ptr<ClientSession> client) {
        // Default message handler just echoes the message back
        std::cout << "Message from client " << client->getClientId() 
                  << " (" << client->getRemoteAddress() << "): " 
                  << message << std::endl;
        client->sendMessage("Echo: " + message);
    };
}

TcpServer::~TcpServer() 
{
    stop();
}

bool TcpServer::start() 
{
    if (running_) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }

    try {
        running_ = true;
        startAccept();
        
        ioThread_ = std::make_unique<std::thread>([this]() {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                std::cerr << "IO thread exception: " << e.what() << std::endl;
                running_ = false;
            }
        });
        
        std::cout << "Server started on port " << port_ << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        running_ = false;
        return false;
    }
}

void TcpServer::stop() 
{
    if (!running_) {
        return;
    }

    running_ = false;
    
    // Stop accepting new connections
    acceptor_.close();
    
    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (auto& pair : clients_) {
            pair.second->stop();
        }
        clients_.clear();
    }
    
    // Stop the io_context
    io_context_.stop();
    
    // Wait for the io_thread to finish
    if (ioThread_ && ioThread_->joinable()) {
        ioThread_->join();
    }
    
    std::cout << "Server stopped" << std::endl;
}

void TcpServer::setMessageHandler(std::function<void(const std::string&, std::shared_ptr<ClientSession>)> handler) 
{
    messageHandler_ = std::move(handler);
}

void TcpServer::broadcastMessage(const std::string& message) 
{
    std::lock_guard<std::mutex> lock(clientsMutex_);
    for (auto& pair : clients_) {
        pair.second->sendMessage(message);
    }
}

size_t TcpServer::getClientCount() const 
{
    std::lock_guard<std::mutex> lock(clientsMutex_);
    return clients_.size();
}

void TcpServer::startAccept() 
{
    if (!running_) {
        return;
    }
    
    auto newClient = std::make_shared<ClientSession>(io_context_, this);
    
    acceptor_.async_accept(
        newClient->socket(),
        [this, newClient](const boost::system::error_code& error) {
            handleAccept(newClient, error);
        }
    );
}

void TcpServer::handleAccept(std::shared_ptr<ClientSession> client, const boost::system::error_code& error)
{
    if (!running_) {
        return;
    }
    
    if (!error) {
        // Check if we're at the maximum number of connections
        if (getClientCount() >= maxConnections_) {
            std::cerr << "Connection rejected: maximum connections reached" << std::endl;
            client->socket().close();
        } else {
            // Add the client and start the session
            size_t clientId = addClient(client);
            client->start(clientId);
            
            std::cout << "New client connected: " << client->getRemoteAddress() 
                      << " (ID: " << clientId << ")" << std::endl;
        }
    } else {
        std::cerr << "Error accepting connection: " << error.message() << std::endl;
    }
    
    // Continue accepting connections
    startAccept();
}

size_t TcpServer::addClient(std::shared_ptr<ClientSession> client) 
{
    std::lock_guard<std::mutex> lock(clientsMutex_);
    size_t clientId = nextClientId_++;
    clients_[clientId] = client;
    return clientId;
}

void TcpServer::removeClient(size_t clientId) 
{
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_.erase(clientId);
}

// ClientSession implementation
ClientSession::ClientSession(boost::asio::io_context& io_context, TcpServer* server):
socket_(io_context),
server_(server),
clientId_(0),
active_(false)
{
}

ClientSession::~ClientSession() 
{
    stop();
}

boost::asio::ip::tcp::socket& ClientSession::socket() 
{
    return socket_;
}

void ClientSession::start(size_t clientId) 
{
    clientId_ = clientId;
    active_ = true;
    
    try {
        // Get the remote address
        auto endpoint = socket_.remote_endpoint();
        remoteAddress_ = endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
        
        // Start reading from the socket
        startRead();
    } catch (const std::exception& e) {
        std::cerr << "Error starting client session: " << e.what() << std::endl;
        stop();
    }
}

void ClientSession::stop() 
{
    if (!active_.exchange(false)) {
        return;  // Already stopped
    }
    
    try {
        if (socket_.is_open()) {
            socket_.close();
        }
        
        // If we have a valid client ID, remove it from the server
        if (clientId_ > 0 && server_) {
            std::cout << "Client disconnected: " << getRemoteAddress() 
                      << " (ID: " << clientId_ << ")" << std::endl;
            server_->removeClient(clientId_);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error stopping client session: " << e.what() << std::endl;
    }
}

void ClientSession::sendMessage(const std::string& message)
{
    if (!active_ || !socket_.is_open()) {
        return;
    }
    
    // Make a copy of the message to ensure it stays valid during the async operation
    auto messageCopy = std::make_shared<std::string>(message);
    
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*messageCopy),
        [this, messageCopy](const boost::system::error_code& error, size_t bytesTransferred) {
            handleWrite(error, bytesTransferred);
        }
    );
}

size_t ClientSession::getClientId() const
{
    return clientId_;
}

std::string ClientSession::getRemoteAddress() const
{
    return remoteAddress_;
}

void ClientSession::startRead()
{
    if (!active_ || !socket_.is_open()) {
        return;
    }
    
    socket_.async_read_some(
        boost::asio::buffer(dataBuffer_, MAX_BUFFER_SIZE),
        [this, self = shared_from_this()](const boost::system::error_code& error, size_t bytesTransferred) {
            handleRead(error, bytesTransferred);
        }
    );
}

void ClientSession::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
{
    if (error) {
        if (error == boost::asio::error::eof) {
            // Client closed the connection normally
            std::cout << "Client " << getClientId() << " closed connection" << std::endl;
        } else {
            std::cerr << "Read error for client " << getClientId() 
                      << ": " << error.message() << std::endl;
        }
        stop();
        return;
    }
    
    if (bytesTransferred > 0 && server_) {
        // Convert the received data to a string
        std::string message(dataBuffer_, bytesTransferred);
        
        // Call the message handler
        server_->messageHandler_(message, shared_from_this());
    }
    
    // Continue reading
    startRead();
}

void ClientSession::handleWrite(const boost::system::error_code& error, size_t bytesTransferred)
{
    if (error) {
        std::cerr << "Write error for client " << getClientId() 
                  << ": " << error.message() << std::endl;
        stop();
    }
}

} // namespace altair
