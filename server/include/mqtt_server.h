#ifndef MQTT_SERVER_H
#define MQTT_SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <map>

class MQTTServer {
public:
    MQTTServer(boost::asio::io_context& io_context, short port);
    
private:
    void start_accept();
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void process_mqtt_packet(std::shared_ptr<boost::asio::ip::tcp::socket> socket, 
                             const std::string& data, 
                             const std::string& client_ip);
    
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    // MQTT topics simulation
    static const std::map<std::string, std::string> mqtt_topics_;
};

#endif // MQTT_SERVER_H




