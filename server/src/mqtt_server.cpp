#include "mqtt_server.h"
#include "soc_logger.h"
#include "rest_api.h"
#include <boost/asio.hpp>
#include <iostream>
#include <cstring>

using boost::asio::ip::tcp;

// Simulated MQTT topics
const std::map<std::string, std::string> MQTTServer::mqtt_topics_ = {
    {"sensors/temperature", "25.5"},
    {"sensors/pressure", "101.3"},
    {"sensors/humidity", "65.0"},
    {"actuators/motor", "ON"},
    {"actuators/valve", "CLOSED"},
    {"system/status", "RUNNING"},
    {"admin/config", "RESTRICTED"},
    {"admin/credentials", "RESTRICTED"}
};

MQTTServer::MQTTServer(boost::asio::io_context& io_context, short port)
    : io_context_(io_context), 
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
    
    start_accept();
    std::cout << "MQTT Server Running on Port " << port << " (Simulated)" << std::endl;
}

void MQTTServer::start_accept() {
    auto socket = std::make_shared<tcp::socket>(io_context_);
    
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            std::string client_ip = socket->remote_endpoint().address().to_string();
            uint16_t port = socket->remote_endpoint().port();
            
            std::cout << "MQTT: Connection from " << client_ip << ":" << port << std::endl;
            SOCLogger::log_mqtt_activity(client_ip, "connection", "CONNECT");
            
            handle_client(socket);
        }
        start_accept();
    });
}

void MQTTServer::handle_client(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<std::vector<char>>(1024);
    
    socket->async_read_some(boost::asio::buffer(*buffer), 
        [this, socket, buffer](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string client_ip = socket->remote_endpoint().address().to_string();
                std::string data(buffer->data(), length);
                
                process_mqtt_packet(socket, data, client_ip);
                
                // Continue reading
                handle_client(socket);
            }
        });
}

void MQTTServer::process_mqtt_packet(std::shared_ptr<tcp::socket> socket, 
                                     const std::string& data, 
                                     const std::string& client_ip) {
    if (data.empty()) return;
    
    // Basic MQTT packet type detection (first byte)
    uint8_t packet_type = (static_cast<uint8_t>(data[0]) >> 4) & 0x0F;
    
    std::string action;
    std::string topic = "unknown";
    
    // Simplified MQTT packet type detection
    switch (packet_type) {
        case 1: // CONNECT
            action = "CONNECT";
            SOCLogger::log_mqtt_activity(client_ip, topic, action);
            RestAPI::increment_mqtt_activities();
            // Send CONNACK
            {
                unsigned char connack[] = {0x20, 0x02, 0x00, 0x00};
                boost::asio::write(*socket, boost::asio::buffer(connack, 4));
            }
            break;
            
        case 3: // PUBLISH
            action = "PUBLISH";
            // Extract topic from packet (simplified)
            if (data.length() > 4) {
                size_t topic_len = (static_cast<uint8_t>(data[2]) << 8) | static_cast<uint8_t>(data[3]);
                if (data.length() >= 4 + topic_len) {
                    topic = data.substr(4, topic_len);
                }
            }
            SOCLogger::log_mqtt_activity(client_ip, topic, action);
            RestAPI::increment_mqtt_activities();
            
            // Detect suspicious topics
            if (topic.find("admin") != std::string::npos || 
                topic.find("credentials") != std::string::npos) {
                SOCLogger::log_attack_with_severity(client_ip, "MQTT_ADMIN_TOPIC_ACCESS", "HIGH");
                RestAPI::increment_attacks();
            }
            break;
            
        case 8: // SUBSCRIBE
            action = "SUBSCRIBE";
            // Extract topic from packet (simplified)
            if (data.length() > 6) {
                size_t topic_len = (static_cast<uint8_t>(data[4]) << 8) | static_cast<uint8_t>(data[5]);
                if (data.length() >= 6 + topic_len) {
                    topic = data.substr(6, topic_len);
                }
            }
            SOCLogger::log_mqtt_activity(client_ip, topic, action);
            RestAPI::increment_mqtt_activities();
            
            // Send SUBACK
            {
                unsigned char suback[] = {0x90, 0x03, (unsigned char)data[2], (unsigned char)data[3], 0x00};
                boost::asio::write(*socket, boost::asio::buffer(suback, 5));
            }
            
            // Detect wildcard subscriptions
            if (topic.find("#") != std::string::npos || topic.find("+") != std::string::npos) {
                SOCLogger::log_attack_with_severity(client_ip, "MQTT_WILDCARD_SUBSCRIPTION", "MEDIUM");
                RestAPI::increment_attacks();
            }
            break;
            
        case 10: // UNSUBSCRIBE
            action = "UNSUBSCRIBE";
            SOCLogger::log_mqtt_activity(client_ip, topic, action);
            RestAPI::increment_mqtt_activities();
            break;
            
        case 12: // PINGREQ
            action = "PING";
            // Send PINGRESP
            {
                unsigned char pingresp[] = {0xD0, 0x00};
                boost::asio::write(*socket, boost::asio::buffer(pingresp, 2));
            }
            break;
            
        case 14: // DISCONNECT
            action = "DISCONNECT";
            SOCLogger::log_mqtt_activity(client_ip, topic, action);
            RestAPI::increment_mqtt_activities();
            break;
            
        default:
            action = "UNKNOWN";
            SOCLogger::log_attack_with_severity(client_ip, "MQTT_MALFORMED_PACKET", "MEDIUM");
            RestAPI::increment_attacks();
            break;
    }
    
    std::cout << "MQTT: " << client_ip << " - " << action;
    if (topic != "unknown") {
        std::cout << " on " << topic;
    }
    std::cout << std::endl;
    
    // Detect large payloads (potential attack)
    if (data.length() > 10240) {  // 10KB
        SOCLogger::log_attack_with_severity(client_ip, "MQTT_LARGE_PAYLOAD", "HIGH");
        RestAPI::increment_attacks();
    }
}

