#ifndef SOC_LOGGER_H
#define SOC_LOGGER_H

#include <string>
#include <fstream>
#include <vector>
#include <mutex>
#include <chrono>

class SOCLogger {
private:
    static std::ofstream log_file_;
    static bool initialized_;
    static std::string log_directory_;
    
    // Intelligent buffering
    static std::vector<std::string> buffer_;
    static std::mutex buffer_mutex_;
    static constexpr size_t BUFFER_SIZE = 10;  // Flush after 10 entries
    static std::chrono::steady_clock::time_point last_flush_;
    static constexpr int FLUSH_INTERVAL_SECONDS = 5;  // Flush every 5 seconds
    
    static void add_to_buffer(const std::string& log_entry);
    static void flush_buffer();

public:
    static void initialize(const std::string& log_file = "honeypot_events.json");
    static void shutdown();
    
    // Specific logging functions
    static void log_modbus_connection(const std::string& client_ip, uint16_t port);
    static void log_coap_request(const std::string& client_ip, const std::string& resource);
    static void log_auth_attempt(const std::string& client_ip, const std::string& protocol, bool success);
    static void log_attack_detected(const std::string& client_ip, const std::string& attack_type);
    static void log_attack_with_severity(const std::string& client_ip, const std::string& attack_type, const std::string& severity);
    static void log_mqtt_activity(const std::string& client_ip, const std::string& topic, const std::string& action);
    
    // Force flush
    static void force_flush();
};

#endif