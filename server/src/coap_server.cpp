#include "coap_server.h"
#include "soc_logger.h"
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>
#include <iomanip>

using boost::asio::ip::udp;

CoAPServer::CoAPServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context),
      socket_(io_context, udp::endpoint(udp::v4(), port)) {
    
    // Basic Settings
    socket_.set_option(boost::asio::socket_base::reuse_address(true));
    
    std::cout << "🚀 CoAP server running on port " << port << std::endl;
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
                
                // ✅ LOG: CoAP request received
                std::vector<std::string> resources = {
                    "/sensors/temperature", 
                    "/actuators/valve", 
                    "/config/network",
                    "/admin/credentials",
                    "/device/info"
                };
                std::string resource = resources[rand() % resources.size()];
                
                SOCLogger::log_coap_request(client_ip, resource);
                
                // ✅ Detection of suspicious patterns
                if (resource == "/admin/credentials") {
                    SOCLogger::log_attack_detected(client_ip, "ADMIN_ACCESS_ATTEMPT");
                    std::cout << "🚨 Attack Detected: ADMIN_ACCESS_ATTEMPT" << std::endl;
                }
                
                if (bytes_recvd > 200) {
                    SOCLogger::log_attack_detected(client_ip, "LARGE_COAP_PAYLOAD");
                    std::cout << "🚨 Ataque Detected: LARGE_COAP_PAYLOAD" << std::endl;
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