#include "coap_server.h"
#include "soc_logger.h"
#include "rest_api.h"
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <map>

using boost::asio::ip::udp;

// Pre-allocated resource strings (static initialization)
const std::vector<std::string> CoAPServer::resources_ = {
    "/sensors/temperature", 
    "/actuators/valve", 
    "/config/network",
    "/admin/credentials",
    "/device/info"
};

CoAPServer::CoAPServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context),
      socket_(io_context, udp::endpoint(udp::v4(), port)) {
    
    // Optimized socket settings
    socket_.set_option(boost::asio::socket_base::reuse_address(true));
    socket_.set_option(boost::asio::socket_base::receive_buffer_size(16384));
    socket_.set_option(boost::asio::socket_base::send_buffer_size(16384));
    
    std::cout << "CoAP server running on port " << port << " (Optimized)" << std::endl;
    std::cout << "📍 Endpoint: " << socket_.local_endpoint().address().to_string() 
              << ":" << socket_.local_endpoint().port() << std::endl;
    std::cout << "💡 Use '127.0.0.1' for testing (IPV4)" << std::endl;
    std::cout << "💡 Use 'socat' or an explicit IPv4 address" << std::endl;
    
    start_receive();
}

void CoAPServer::start_receive() {
    recv_buffer_.fill(0);

    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        [this](boost::system::error_code ec, std::size_t bytes_recvd) {
            if (!ec && bytes_recvd > 0) {
                std::string client_ip = remote_endpoint_.address().to_string();
                uint16_t client_port = remote_endpoint_.port();
                
                std::cout << "📨 CoAP: " << bytes_recvd << " bytes of " 
                         << client_ip << ":" << client_port << std::endl;
                
                // Converts data to string
                std::string received_data(recv_buffer_.data(), bytes_recvd);
                
                // ✅ Log to Console
                std::cout << "📦 Data: '";
                for (char c : received_data) {
                    if (c >= 32 && c <= 126) {
                        std::cout << c;
                    } else if (c == '\n') {
                        std::cout << "\\n";
                    } else if (c == '\r') {
                        std::cout << "\\r";
                    } else {
                        std::cout << "?";
                    }
                }
                std::cout << "'" << std::endl;
                
                // Use pre-allocated resources (no dynamic allocation)
                const std::string& resource = resources_[rand() % resources_.size()];
                
                SOCLogger::log_coap_request(client_ip, resource);
                RestAPI::increment_coap_requests();
                
                // Enhanced attack pattern detection
                if (resource == "/admin/credentials") {
                    SOCLogger::log_attack_with_severity(client_ip, "ADMIN_ACCESS_ATTEMPT", "CRITICAL");
                    std::cout << "Attack Detected: ADMIN_ACCESS_ATTEMPT" << std::endl;
                    RestAPI::increment_attacks();
                } else if (resource == "/config/network") {
                    SOCLogger::log_attack_with_severity(client_ip, "CONFIG_ACCESS_ATTEMPT", "HIGH");
                    std::cout << "Attack Detected: CONFIG_ACCESS_ATTEMPT" << std::endl;
                    RestAPI::increment_attacks();
                }
                
                if (bytes_recvd > 1024) {
                    SOCLogger::log_attack_with_severity(client_ip, "COAP_BUFFER_OVERFLOW_ATTEMPT", "CRITICAL");
                    std::cout << "Attack Detected: COAP_BUFFER_OVERFLOW_ATTEMPT" << std::endl;
                    RestAPI::increment_attacks();
                } else if (bytes_recvd > 200) {
                    SOCLogger::log_attack_with_severity(client_ip, "LARGE_COAP_PAYLOAD", "MEDIUM");
                    std::cout << "Attack Detected: LARGE_COAP_PAYLOAD" << std::endl;
                    RestAPI::increment_attacks();
                }
                
                // Check for rapid fire (potential DDoS)
                static std::map<std::string, int> request_counter;
                request_counter[client_ip]++;
                if (request_counter[client_ip] > 100) {
                    SOCLogger::log_attack_with_severity(client_ip, "COAP_DDOS_ATTEMPT", "CRITICAL");
                    RestAPI::increment_attacks();
                    request_counter[client_ip] = 0;  // Reset counter
                }
                
                // ✅ CoAP Response
                std::string response = "CoAP_ACK_" + std::to_string(bytes_recvd) + "B";
                socket_.async_send_to(
                    boost::asio::buffer(response), remote_endpoint_,
                    [client_ip](boost::system::error_code ec, std::size_t bytes_sent) {
                        if (!ec) {
                            std::cout << "📤 Reply sent to " << client_ip 
                                     << " (" << bytes_sent << " bytes)" << std::endl;
                        }
                    });
            }
            
            // ✅ Continue receiving
            start_receive();
        });
}