#ifndef SECURITY_H
#define SECURITY_H

#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <algorithm>

// Security utilities for honeypot
class Security {
public:
    // Sanitize string for logging (prevent log injection)
    static std::string sanitize_for_log(const std::string& input);
    
    // Validate IP address format
    static bool is_valid_ip(const std::string& ip);
    
    // Rate limiting per IP
    static bool check_rate_limit(const std::string& ip, int max_requests_per_minute);
    
    // Sanitize path (prevent directory traversal)
    static std::string sanitize_path(const std::string& path);
    
    // Validate buffer size
    static bool is_safe_buffer_size(size_t size, size_t max_size);
    
private:
    static std::map<std::string, std::chrono::steady_clock::time_point> rate_limit_map_;
    static std::map<std::string, int> request_count_map_;
    static std::mutex rate_limit_mutex_;
    
    static constexpr size_t MAX_LOG_STRING_LENGTH = 1024;
    static constexpr size_t MAX_PATH_LENGTH = 256;
};

#endif // SECURITY_H

