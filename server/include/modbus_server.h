#ifndef MODBUS_SERVER_H
#define MODBUS_SERVER_H

#include <boost/asio.hpp>
#include <memory>

class ModbusServer {
public:
    ModbusServer(boost::asio::io_context& io_context, short port);
    
private:
    void start_accept();
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif // MODBUS_SERVER_H