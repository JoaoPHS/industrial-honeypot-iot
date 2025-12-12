#include "soc_logger.h"
#include "security.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <sys/stat.h>
#include <unistd.h>

// Initialization of static variables
std::ofstream SOCLogger::log_file_;
bool SOCLogger::initialized_ = false;
std::string SOCLogger::log_directory_ = "";

// Intelligent buffering variables
std::vector<std::string> SOCLogger::buffer_;
std::mutex SOCLogger::buffer_mutex_;
std::chrono::steady_clock::time_point SOCLogger::last_flush_ = std::chrono::steady_clock::now();

static std::mutex time_mutex;

static std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::lock_guard<std::mutex> lock(time_mutex);
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
    return std::string(buffer);
}

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
        std::cerr << "Error opening log file" << std::endl;
        return;
    }
    
    // Reserve buffer space for better performance
    buffer_.reserve(BUFFER_SIZE);
    last_flush_ = std::chrono::steady_clock::now();
    
    initialized_ = true;
    log_file_ << "[" << std::endl;
    std::cout << "SOC_Logger Ready (Buffered Mode): " << full_path << std::endl;
}

void SOCLogger::shutdown() {
    // Flush remaining buffer before closing
    flush_buffer();
    
    if (log_file_.is_open()) {
        log_file_ << "\n]" << std::endl;
        log_file_.close();
    }
    initialized_ = false;
}

// Add log entry to buffer
void SOCLogger::add_to_buffer(const std::string& log_entry) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    buffer_.push_back(log_entry);
    
    // Check if we need to flush (by size or time)
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_flush_).count();
    
    if (buffer_.size() >= BUFFER_SIZE || elapsed >= FLUSH_INTERVAL_SECONDS) {
        flush_buffer();
    }
}

// Flush buffer to disk
void SOCLogger::flush_buffer() {
    if (!initialized_ || buffer_.empty()) return;
    
    for (const auto& entry : buffer_) {
        log_file_ << entry;
    }
    log_file_.flush();
    
    buffer_.clear();
    last_flush_ = std::chrono::steady_clock::now();
}

// Force immediate flush
void SOCLogger::force_flush() {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    flush_buffer();
}

void SOCLogger::log_modbus_connection(const std::string& client_ip, uint16_t port) {
    if (!initialized_) return;
    
    std::string timestamp = get_timestamp();
    std::string sanitized_ip = Security::sanitize_for_log(client_ip);
    std::string sanitized_timestamp = Security::sanitize_for_log(timestamp);
    
    std::ostringstream log_entry;
    log_entry << "  {\n"
              << "    \"timestamp\": \"" << sanitized_timestamp << "\",\n"
              << "    \"event_type\": \"MODBUS_CONNECTION\",\n"
              << "    \"client_ip\": \"" << sanitized_ip << "\",\n"
              << "    \"port\": " << port << ",\n"
              << "    \"message\": \"Modbus connection established\"\n"
              << "  },\n";
    
    add_to_buffer(log_entry.str());
    
    std::cout << "📡 Modbus: " << client_ip << ":" << port << std::endl;
}

void SOCLogger::log_coap_request(const std::string& client_ip, const std::string& resource) {
    if (!initialized_) return;
    
    std::string timestamp = get_timestamp();
    std::string sanitized_ip = Security::sanitize_for_log(client_ip);
    std::string sanitized_resource = Security::sanitize_for_log(resource);
    std::string sanitized_timestamp = Security::sanitize_for_log(timestamp);
    
    std::ostringstream log_entry;
    log_entry << "  {\n"
              << "    \"timestamp\": \"" << sanitized_timestamp << "\",\n"
              << "    \"event_type\": \"COAP_REQUEST\",\n"
              << "    \"client_ip\": \"" << sanitized_ip << "\",\n"
              << "    \"resource\": \"" << sanitized_resource << "\",\n"
              << "    \"message\": \"CoAP Request Received\"\n"
              << "  },\n";
    
    add_to_buffer(log_entry.str());
    
    std::cout << "📱 CoAP: " << client_ip << " -> " << resource << std::endl;
}

void SOCLogger::log_auth_attempt(const std::string& client_ip, const std::string& protocol, bool success) {
    if (!initialized_) return;
    
    std::string timestamp = get_timestamp();
    std::string status = success ? "SUCCESS" : "FAILED";
    std::string sanitized_ip = Security::sanitize_for_log(client_ip);
    std::string sanitized_protocol = Security::sanitize_for_log(protocol);
    std::string sanitized_status = Security::sanitize_for_log(status);
    std::string sanitized_timestamp = Security::sanitize_for_log(timestamp);
    
    std::ostringstream log_entry;
    log_entry << "  {\n"
              << "    \"timestamp\": \"" << sanitized_timestamp << "\",\n"
              << "    \"event_type\": \"AUTH_ATTEMPT\",\n"
              << "    \"client_ip\": \"" << sanitized_ip << "\",\n"
              << "    \"protocol\": \"" << sanitized_protocol << "\",\n"
              << "    \"status\": \"" << sanitized_status << "\",\n"
              << "    \"message\": \"Authentication Attempt " << sanitized_status << "\"\n"
              << "  },\n";
    
    add_to_buffer(log_entry.str());
    
    std::cout << "🔐 Auth " << protocol << ": " << client_ip << " - " << status << std::endl;
}
void SOCLogger::log_attack_detected(const std::string& client_ip, const std::string& attack_type) {
    if (!initialized_) return;
    
    std::string timestamp = get_timestamp();
    std::string sanitized_ip = Security::sanitize_for_log(client_ip);
    std::string sanitized_attack = Security::sanitize_for_log(attack_type);
    std::string sanitized_timestamp = Security::sanitize_for_log(timestamp);
    
    std::ostringstream log_entry;
    log_entry << "  {\n"
              << "    \"timestamp\": \"" << sanitized_timestamp << "\",\n"
              << "    \"event_type\": \"ATTACK_DETECTED\",\n"
              << "    \"client_ip\": \"" << sanitized_ip << "\",\n"
              << "    \"attack_type\": \"" << sanitized_attack << "\",\n"
              << "    \"severity\": \"HIGH\",\n"
              << "    \"message\": \"Possible Attack Detected: " << sanitized_attack << "\"\n"
              << "  },\n";
    
    add_to_buffer(log_entry.str());
    
    std::cout << "ATTACK: " << attack_type << " de " << client_ip << std::endl;
}

void SOCLogger::log_attack_with_severity(const std::string& client_ip, const std::string& attack_type, const std::string& severity) {
    if (!initialized_) return;
    
    std::string timestamp = get_timestamp();
    std::string sanitized_ip = Security::sanitize_for_log(client_ip);
    std::string sanitized_attack = Security::sanitize_for_log(attack_type);
    std::string sanitized_severity = Security::sanitize_for_log(severity);
    std::string sanitized_timestamp = Security::sanitize_for_log(timestamp);
    
    std::ostringstream log_entry;
    log_entry << "  {\n"
              << "    \"timestamp\": \"" << sanitized_timestamp << "\",\n"
              << "    \"event_type\": \"ATTACK_DETECTED\",\n"
              << "    \"client_ip\": \"" << sanitized_ip << "\",\n"
              << "    \"attack_type\": \"" << sanitized_attack << "\",\n"
              << "    \"severity\": \"" << sanitized_severity << "\",\n"
              << "    \"message\": \"Attack Detected: " << sanitized_attack << " [" << sanitized_severity << "]\"\n"
              << "  },\n";
    
    add_to_buffer(log_entry.str());
    
    std::cout << "ATTACK [" << severity << "]: " << attack_type << " from " << client_ip << std::endl;
}

void SOCLogger::log_mqtt_activity(const std::string& client_ip, const std::string& topic, const std::string& action) {
    if (!initialized_) return;
    
    std::string timestamp = get_timestamp();
    std::string sanitized_ip = Security::sanitize_for_log(client_ip);
    std::string sanitized_topic = Security::sanitize_for_log(topic);
    std::string sanitized_action = Security::sanitize_for_log(action);
    std::string sanitized_timestamp = Security::sanitize_for_log(timestamp);
    
    std::ostringstream log_entry;
    log_entry << "  {\n"
              << "    \"timestamp\": \"" << sanitized_timestamp << "\",\n"
              << "    \"event_type\": \"MQTT_ACTIVITY\",\n"
              << "    \"client_ip\": \"" << sanitized_ip << "\",\n"
              << "    \"topic\": \"" << sanitized_topic << "\",\n"
              << "    \"action\": \"" << sanitized_action << "\",\n"
              << "    \"message\": \"MQTT " << sanitized_action << " on topic: " << sanitized_topic << "\"\n"
              << "  },\n";
    
    add_to_buffer(log_entry.str());
    
    std::cout << "MQTT: " << client_ip << " - " << action << " on " << topic << std::endl;
}