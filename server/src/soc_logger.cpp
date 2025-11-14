#include "soc_logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>

// Initialization of static variables
std::ofstream SOCLogger::log_file_;
bool SOCLogger::initialized_ = false;
std::string SOCLogger::log_directory_ = "";

void SOCLogger::initialize(const std::string& log_file) {
    std::cout << "🔧 Initializing SOC_Logger..." << std::endl;
    
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        log_directory_ = std::string(cwd) + "/logs/";
    } else {
        log_directory_ = "./logs/";
    }
    
    mkdir(log_directory_.c_str(), 0755);
    
    std::string full_path = log_directory_ + log_file;
    log_file_.open(full_path, std::ios::app);
    
    if (!log_file_.is_open()) {
        std::cerr << "❌ Error opening log file" << std::endl;
        return;
    }
    
    initialized_ = true;
    log_file_ << "[" << std::endl;
    std::cout << "✅ SOC_Logger Ready: " << full_path << std::endl;
}

void SOCLogger::shutdown() {
    if (log_file_.is_open()) {
        log_file_ << "\n]" << std::endl;
        log_file_.close();
    }
    initialized_ = false;
}

void SOCLogger::log_modbus_connection(const std::string& client_ip, uint16_t port) {
    if (!initialized_) return;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&time_t);
    timestamp.erase(timestamp.find('\n'), 1);
    
    log_file_ << "  {\n"
              << "    \"timestamp\": \"" << timestamp << "\",\n"
              << "    \"event_type\": \"MODBUS_CONNECTION\",\n"
              << "    \"client_ip\": \"" << client_ip << "\",\n"
              << "    \"port\": " << port << ",\n"
              << "    \"message\": \"Modbus connection established\"\n"
              << "  }," << std::endl;
    log_file_.flush();
    
    std::cout << "📡 Modbus: " << client_ip << ":" << port << std::endl;
}

void SOCLogger::log_coap_request(const std::string& client_ip, const std::string& resource) {
    if (!initialized_) return;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&time_t);
    timestamp.erase(timestamp.find('\n'), 1);
    
    log_file_ << "  {\n"
              << "    \"timestamp\": \"" << timestamp << "\",\n"
              << "    \"event_type\": \"COAP_REQUEST\",\n"
              << "    \"client_ip\": \"" << client_ip << "\",\n"
              << "    \"resource\": \"" << resource << "\",\n"
              << "    \"message\": \"CoAP Request Received\"\n"
              << "  }," << std::endl;
    log_file_.flush();
    
    std::cout << "📱 CoAP: " << client_ip << " -> " << resource << std::endl;
}

void SOCLogger::log_auth_attempt(const std::string& client_ip, const std::string& protocol, bool success) {
    if (!initialized_) return;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&time_t);
    timestamp.erase(timestamp.find('\n'), 1);
    
    std::string status = success ? "SUCCESS" : "FAILED";
    
    log_file_ << "  {\n"
              << "    \"timestamp\": \"" << timestamp << "\",\n"
              << "    \"event_type\": \"AUTH_ATTEMPT\",\n"
              << "    \"client_ip\": \"" << client_ip << "\",\n"
              << "    \"protocol\": \"" << protocol << "\",\n"
              << "    \"status\": \"" << status << "\",\n"
              << "    \"message\": \"Authentication Attempt \" + status + \"\"\n"
              << "  }," << std::endl;
    log_file_.flush();
    
    std::cout << "🔐 Auth " << protocol << ": " << client_ip << " - " << status << std::endl;
}
void SOCLogger::log_attack_detected(const std::string& client_ip, const std::string& attack_type) {
    if (!initialized_) return;
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&time_t);
    timestamp.erase(timestamp.find('\n'), 1);
    
    log_file_ << "  {\n"
              << "    \"timestamp\": \"" << timestamp << "\",\n"
              << "    \"event_type\": \"ATTACK_DETECTED\",\n"
              << "    \"client_ip\": \"" << client_ip << "\",\n"
              << "    \"attack_type\": \"" << attack_type << "\",\n"
              << "    \"severity\": \"HIGH\",\n"
              << "    \"message\": \"Possible Attack Detected: \" + attack_type + \"\"\n"
              << "  }," << std::endl;
    log_file_.flush();
    
    std::cout << "🚨 ATTACK: " << attack_type << " de " << client_ip << std::endl;
}