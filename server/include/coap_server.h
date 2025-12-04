#ifndef COAP_SERVER_H
#define COAP_SERVER_H

#include <boost/asio.hpp>
#include <array>
#include <vector>
#include <string>

class CoAPServer {
public:
    CoAPServer(boost::asio::io_context& io_context, short port);
    
private:
    void start_receive();

    boost::asio::io_context& io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    
    // Larger buffer for better performance
    std::array<char, 2048> recv_buffer_;
    
    // Pre-allocated resource strings
    static const std::vector<std::string> resources_;
};

#endif // COAP_SERVER_H