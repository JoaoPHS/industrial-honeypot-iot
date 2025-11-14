#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <iomanip>
#include <sstream>
#include "soc_logger.h"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

std::atomic<bool> shutdown_requested{false};

void signal_handler(int signum) {
    (void)signum;
    std::cout << "\n🛑 Interrupt signal received" << std::endl;
    shutdown_requested = true;
}

class DebugTCPServer {
private:
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;

public:
    DebugTCPServer(boost::asio::io_context& io_context, short port)
        : io_context_(io_context), 
          acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        
        std::cout << "🎯 DebugTCPServer created on port " << port << std::endl;
        
        if (acceptor_.is_open()) {
            std::cout << "✅ Acceptor OPENED successfully" << std::endl;
            std::cout << "📍 Endpoint: " << acceptor_.local_endpoint().address().to_string() 
                      << ":" << acceptor_.local_endpoint().port() << std::endl;
        } else {
            std::cerr << "❌ ERROR: Acceptor CLOSED" << std::endl;
        }
        
        start_accept();
    }

private:
    void start_accept() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        
        std::cout << "🔧 Configuring async_accept..." << std::endl;
        
        acceptor_.async_accept(*socket, 
            [this, socket](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "🎉 TCP CONNECTION ACCEPTED FROM " 
                             << socket->remote_endpoint().address().to_string() 
                             << ":" << socket->remote_endpoint().port() << std::endl;
                    
                    // Log in to SOC Logger
                    std::map<std::string, std::string> details;
                    details["protocol"] = "TCP";
                    details["port"] = std::to_string(acceptor_.local_endpoint().port());
                    details["event"] = "CONNECTION_ACCEPTED";
                    
                    SOCLogger::logEvent(SOCLogger::Severity::LOW, "DEBUG_TCP", 
                                       socket->remote_endpoint().address().to_string(), 
                                       "DEBUG_CONNECTION", details);
                    
                    // Send simple response
                    std::string response = "DEBUG_SERVER: Connection accepted\n";
                    boost::asio::write(*socket, boost::asio::buffer(response));
                    
                    socket->close();
                } else {
                    std::cerr << "❌ Accept Error: " << ec.message() << std::endl;
                }
                
                // Accept next connection
                start_accept();
            });
        
        std::cout << "✅ Async_accept configured - Waiting connections..." << std::endl;
    }
};

class DebugUDPServer {
private:
    boost::asio::io_context& io_context_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_;

public:
    DebugUDPServer(boost::asio::io_context& io_context, short port)
        : io_context_(io_context), 
          socket_(io_context, udp::endpoint(udp::v4(), port)) {
        
        std::cout << "🎯 DebugUDPServer created on port " << port << std::endl;
        
        if (socket_.is_open()) {
            std::cout << "✅ UDP socket opened successfully" << std::endl;
            std::cout << "📍 Endpoint: " << socket_.local_endpoint().address().to_string() 
                      << ":" << socket_.local_endpoint().port() << std::endl;
        } else {
            std::cerr << "❌ ERROR: UDP socket closed" << std::endl;
        }
        
        start_receive();
    }

private:
    void start_receive() {
        std::cout << "🔧 Configuring async_receive_from..." << std::endl;
        
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::cout << "🎉 UDP datagram received from " 
                             << remote_endpoint_.address().to_string() 
                             << ":" << remote_endpoint_.port() 
                             << " (" << bytes_recvd << " bytes)" << std::endl;
                    
                    // Log in to SOC Logger
                    std::map<std::string, std::string> details;
                    details["protocol"] = "UDP";
                    details["port"] = std::to_string(socket_.local_endpoint().port());
                    details["bytes_received"] = std::to_string(bytes_recvd);
                    details["data_hex"] = bytes_to_hex(recv_buffer_.data(), bytes_recvd);
                    
                    SOCLogger::logEvent(SOCLogger::Severity::LOW, "DEBUG_UDP", 
                                       remote_endpoint_.address().to_string(), 
                                       "DEBUG_DATAGRAM", details);
                    
                    // Send simple response
                    std::string response = "DEBUG_SERVER: Datagram received\n";
                    socket_.async_send_to(
                        boost::asio::buffer(response), remote_endpoint_,
                        [](boost::system::error_code, std::size_t) {});
                } else if (ec) {
                    std::cerr << "❌ Receive error: " << ec.message() << std::endl;
                }
                
                start_receive();
            });
        
        std::cout << "✅ Async_receive_from configured - Waiting datagrams..." << std::endl;
    }
    
    std::string bytes_to_hex(const char* data, size_t length) {
        std::stringstream ss;
        for (size_t i = 0; i < length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << (static_cast<unsigned int>(data[i]) & 0xFF) << " ";
        }
        return ss.str();
    }
};

int main() {
    std::signal(SIGINT, signal_handler);
    
    std::cout << "🐛 STARTING DEBUG SERVER WITH DEFAULT PORTS" << std::endl;
    
    try {
        SOCLogger::initialize("debug_server.json");
        std::cout << "✅ Logger initialized" << std::endl;
        
        boost::asio::io_context io_context;
        
        // ✅ STANDARD PORTS - ROOT ACCESS REQUIRED
        DebugTCPServer tcp_server(io_context, 502);    // Standard Modbus
        DebugUDPServer udp_server(io_context, 5683);   // Standard CoAP
        
        std::cout << "\n🎉 ALL DEBUG SERVERS STARTED!" << std::endl;
        std::cout << "📍 Modbus TCP:502 | CoAP UDP:5683" << std::endl;
        std::cout << "⏳ Waiting for connections...\n" << std::endl;
        
        // Timer checks if the io_context is operational
        boost::asio::steady_timer timer(io_context);
        std::function<void()> check_io_context;
        check_io_context = [&]() {
            timer.expires_after(std::chrono::seconds(5));
            timer.async_wait([&](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "💓 Active io_context - still processing..." << std::endl;
                    check_io_context();
                }
            });
        };
        check_io_context();
        
        // Executes the io_context in a separate thread
        std::thread io_thread([&io_context]() {
            std::cout << "🧵 Thread io_context started" << std::endl;
            try {
                io_context.run();
                std::cout << "🧵 Thread io_context completed (run() returned)" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "❌ EXCEPT in the thread io_context: " << e.what() << std::endl;
            }
        });
        
        // Main loop
        int counter = 0;
        while (!shutdown_requested) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            counter++;
            
            if (counter % 10 == 0) {
                std::cout << "⏰ Active server for " << counter << " seconds" << std::endl;
            }
        }
        
        std::cout << "🛑 Stopping io_context..." << std::endl;
        io_context.stop();
        
        if (io_thread.joinable()) {
            io_thread.join();
            std::cout << "✅ Thread io_context completed" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "💥 FATAL ERROR: " << e.what() << std::endl;
    }
    
    SOCLogger::shutdown();
    std::cout << "👋 Debug Server complete" << std::endl;
    return 0;
}