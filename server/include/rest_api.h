#ifndef REST_API_H
#define REST_API_H

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <map>
#include <atomic>

class RestAPI {
public:
    RestAPI(boost::asio::io_context& io_context, short port);
    
    // Statistics tracking
    static void increment_modbus_connections();
    static void increment_coap_requests();
    static void increment_mqtt_activities();
    static void increment_attacks();
    
    static std::map<std::string, int> get_statistics();
    
private:
    void start_accept();
    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void process_http_request(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                              const std::string& request);
    
    std::string generate_stats_json();
    std::string generate_http_response(const std::string& content_type,
                                      const std::string& body);
    
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    
    // Statistics counters
    static std::atomic<int> modbus_connections_;
    static std::atomic<int> coap_requests_;
    static std::atomic<int> mqtt_activities_;
    static std::atomic<int> attacks_detected_;
};

#endif // REST_API_H




