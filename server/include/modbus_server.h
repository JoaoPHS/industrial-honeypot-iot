#ifndef MODBUS_SERVER_H
#define MODBUS_SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>

class ModbusServer {
public:
    ModbusServer(boost::asio::io_context& io_context, short port);
    
private:
    void start_accept();
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    
    // Connection pool for socket reuse
    void setup_socket_options(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    
    // Buffer pool for efficient memory allocation
    std::shared_ptr<std::vector<char>> get_buffer();
    void return_buffer(std::shared_ptr<std::vector<char>> buffer);
    
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    // Buffer pool
    std::queue<std::shared_ptr<std::vector<char>>> buffer_pool_;
    std::mutex buffer_pool_mutex_;
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr size_t MAX_POOL_SIZE = 50;
};

#endif // MODBUS_SERVER_H