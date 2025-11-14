#ifndef SOC_LOGGER_H
#define SOC_LOGGER_H

#include <string>
#include <fstream>

class SOCLogger {
private:
    static std::ofstream log_file_;
    static bool initialized_;
    static std::string log_directory_;

public:
    static void initialize(const std::string& log_file = "honeypot_events.json");
    static void shutdown();
    
    // Specific logging functions
    static void log_modbus_connection(const std::string& client_ip, uint16_t port);
    static void log_coap_request(const std::string& client_ip, const std::string& resource);
    static void log_auth_attempt(const std::string& client_ip, const std::string& protocol, bool success);
    static void log_attack_detected(const std::string& client_ip, const std::string& attack_type);
};

#endif