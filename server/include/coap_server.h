#ifndef COAP_SERVER_H
#define COAP_SERVER_H

#include <boost/asio.hpp>
#include <array>

class CoAPServer {
public:
    CoAPServer(boost::asio::io_context& io_context, short port);
    
private:
    void start_receive();

    boost::asio::io_context& io_context_;
    boost::asio::ip::udp::socket socket_;
    boost::asio::ip::udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_;
};

#endif // COAP_SERVER_H