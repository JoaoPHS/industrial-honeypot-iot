#include "modbus_server.h"
#include "soc_logger.h"
#include "rest_api.h"
#include "security.h"
#include <boost/asio.hpp>
#include <iostream>
#include <random>
#include <mutex>
#include <chrono>

// ✅ Use namespaces to avoid 'TCP' errors.
using boost::asio::ip::tcp;

// ✅ Initializes io_context in the initialization list
ModbusServer::ModbusServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context), 
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    
    // Optimize acceptor for connection reuse
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
    
    start_accept();
    std::cout << "Modbus Server Running on Port " << port << " (Optimized)" << std::endl;
}

// Setup socket options for better performance
void ModbusServer::setup_socket_options(std::shared_ptr<tcp::socket> socket) {
    // Enable TCP keep-alive
    socket->set_option(boost::asio::socket_base::keep_alive(true));
    
    // Disable Nagle's algorithm for lower latency
    socket->set_option(tcp::no_delay(true));
    
    // Set receive buffer size
    socket->set_option(boost::asio::socket_base::receive_buffer_size(8192));
    
    // Set send buffer size
    socket->set_option(boost::asio::socket_base::send_buffer_size(8192));
}

// Get buffer from pool (or create new one)
std::shared_ptr<std::vector<char>> ModbusServer::get_buffer() {
    std::lock_guard<std::mutex> lock(buffer_pool_mutex_);
    
    if (!buffer_pool_.empty()) {
        auto buffer = buffer_pool_.front();
        buffer_pool_.pop();
        return buffer;
    }
    
    // Create new buffer if pool is empty
    auto buffer = std::make_shared<std::vector<char>>(BUFFER_SIZE);
    return buffer;
}

void ModbusServer::return_buffer(std::shared_ptr<std::vector<char>> buffer) {
    std::lock_guard<std::mutex> lock(buffer_pool_mutex_);
    
    static std::chrono::steady_clock::time_point last_cleanup = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    
    if (std::chrono::duration_cast<std::chrono::minutes>(now - last_cleanup).count() >= 5) {
        while (buffer_pool_.size() > MAX_POOL_SIZE / 2) {
            buffer_pool_.pop();
        }
        last_cleanup = now;
    }
    
    if (buffer_pool_.size() < MAX_POOL_SIZE) {
        buffer->clear();
        buffer->resize(BUFFER_SIZE);
        buffer_pool_.push(std::move(buffer));
    }
}

void ModbusServer::start_accept() {
    // ✅ Use explicit type and create socket correctly
    auto socket = std::make_shared<tcp::socket>(io_context_);
    
    // ✅ Captures socket correctly in lambda
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            // Apply socket optimizations
            setup_socket_options(socket);
            
            std::string client_ip;
            uint16_t port = 0;
            try {
                client_ip = socket->remote_endpoint().address().to_string();
                port = socket->remote_endpoint().port();
                if (!Security::is_valid_ip(client_ip)) {
                    client_ip = "INVALID_IP";
                }
            } catch (const std::exception& e) {
                client_ip = "UNKNOWN";
            }
            
            SOCLogger::log_modbus_connection(client_ip, port);
            RestAPI::increment_modbus_connections();
            
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::mutex rand_mutex;
            bool auth_success = [&]() {
                std::lock_guard<std::mutex> lock(rand_mutex);
                std::uniform_int_distribution<> dis(0, 1);
                return dis(gen) == 0;
            }();
            SOCLogger::log_auth_attempt(client_ip, "MODBUS", auth_success);
            
            // If authentication fails, it simulates an attack
            if (!auth_success) {
                SOCLogger::log_attack_detected(client_ip, "UNAUTHORIZED_ACCESS");
            }
            
            handle_client(socket);
        }
        start_accept();
    });
}

void ModbusServer::handle_client(std::shared_ptr<tcp::socket> socket) {
    // Get buffer from pool
    auto buffer = get_buffer();
    
    // ✅ Capture all parameters correctly
    socket->async_read_some(boost::asio::buffer(*buffer), 
        [this, socket, buffer](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                if (!Security::is_safe_buffer_size(length, BUFFER_SIZE)) {
                    return_buffer(buffer);
                    return;
                }
                
                std::string client_ip;
                try {
                    client_ip = socket->remote_endpoint().address().to_string();
                    if (!Security::is_valid_ip(client_ip)) {
                        client_ip = "INVALID_IP";
                    }
                } catch (const std::exception& e) {
                    client_ip = "UNKNOWN";
                }
                
                if (length > 512) {
                    SOCLogger::log_attack_with_severity(client_ip, "MODBUS_BUFFER_OVERFLOW_ATTEMPT", "CRITICAL");
                    RestAPI::increment_attacks();
                } else if (length > 100) {
                    SOCLogger::log_attack_with_severity(client_ip, "MODBUS_LARGE_PACKET", "MEDIUM");
                    RestAPI::increment_attacks();
                }
                
                // Check for scanning patterns (very small packets)
                if (length < 6) {
                    SOCLogger::log_attack_with_severity(client_ip, "MODBUS_PORT_SCAN", "LOW");
                    RestAPI::increment_attacks();
                }
                
                // Simulates Modbus response (pre-allocated)
                static const std::string response = "\x00\x01\x00\x00\x00\x06\x01\x03\x00\x00\x00\x01";
                boost::asio::write(*socket, boost::asio::buffer(response));
                
                // Return buffer to pool
                return_buffer(buffer);
                
                // Keep on reading
                handle_client(socket);
            } else {
                // Return buffer to pool on error
                return_buffer(buffer);
            }
        });
}