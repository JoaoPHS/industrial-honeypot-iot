#include "modbus_server.h"
#include "soc_logger.h"
#include <boost/asio.hpp>
#include <iostream>

// ✅ Use namespaces to avoid 'TCP' errors.
using boost::asio::ip::tcp;

// ✅ Initializes io_context in the initialization list
ModbusServer::ModbusServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context), 
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
    std::cout << "🚀 Modbus Server Running on Port " << port << std::endl;
}

void ModbusServer::start_accept() {
    // ✅ Use explicit type and create socket correctly
    auto socket = std::make_shared<tcp::socket>(io_context_);
    
    // ✅ Captures socket correctly in lambda
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            // ✅ LOG: Modbus new connection
            std::string client_ip = socket->remote_endpoint().address().to_string();
            uint16_t port = socket->remote_endpoint().port();
            SOCLogger::log_modbus_connection(client_ip, port);
            
            // Simulate authentication (demonstration)
            bool auth_success = (rand() % 2) == 0;
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
    // ✅ Use explicit type for buffer
    auto buffer = std::make_shared<std::vector<char>>(1024);
    
    // ✅ Capture all parameters correctly
    socket->async_read_some(boost::asio::buffer(*buffer), 
        [this, socket, buffer](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string client_ip = socket->remote_endpoint().address().to_string();
                
                // ✅ Modbus operation log based on packet size
                if (length > 100) {
                    SOCLogger::log_attack_detected(client_ip, "MODBUS_LARGE_PACKET");
                }
                
                // Simulates Modbus response
                std::string response = "\x00\x01\x00\x00\x00\x06\x01\x03\x00\x00\x00\x01";
                boost::asio::write(*socket, boost::asio::buffer(response));
                
                // Keep on reading
                handle_client(socket);
            }
        });
}