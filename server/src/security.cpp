#include "../include/security.h"
#include <regex>
#include <sstream>

// Initialize static members
std::map<std::string, std::chrono::steady_clock::time_point> Security::rate_limit_map_;
std::map<std::string, int> Security::request_count_map_;
std::mutex Security::rate_limit_mutex_;

// Sanitize string for logging - prevent log injection attacks
std::string Security::sanitize_for_log(const std::string& input) {
    if (input.empty()) return "";
    
    // Truncate if too long
    std::string sanitized = input.substr(0, MAX_LOG_STRING_LENGTH);
    
    // Replace dangerous characters
    std::string result;
    result.reserve(sanitized.length());
    
    for (char c : sanitized) {
        // Allow only printable ASCII and common safe characters
        if (c >= 32 && c <= 126 && c != '"' && c != '\\' && c != '\n' && c != '\r') {
            result += c;
        } else {
            // Replace with escaped representation
            result += '_';
        }
    }
    
    return result;
}

// Validate IP address format (prevent injection)
bool Security::is_valid_ip(const std::string& ip) {
    // IPv4 regex pattern
    static const std::regex ipv4_pattern(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    
    // IPv6 simplified check (basic validation)
    static const std::regex ipv6_pattern("^([0-9a-fA-F]{0,4}:){2,7}[0-9a-fA-F]{0,4}$");
    
    return std::regex_match(ip, ipv4_pattern) || std::regex_match(ip, ipv6_pattern);
}

// Rate limiting per IP - prevent DDoS
bool Security::check_rate_limit(const std::string& ip, int max_requests_per_minute) {
    std::lock_guard<std::mutex> lock(rate_limit_mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto& last_time = rate_limit_map_[ip];
    auto& count = request_count_map_[ip];
    
    // Calculate time difference in seconds
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_time).count();
    
    // Reset counter if more than 60 seconds have passed
    if (duration >= 60) {
        last_time = now;
        count = 1;
        return true;
    }
    
    // Increment counter
    count++;
    
    // Check if exceeded limit
    if (count > max_requests_per_minute) {
        return false; // Rate limit exceeded
    }
    
    return true;
}

// Sanitize path - prevent directory traversal attacks
std::string Security::sanitize_path(const std::string& path) {
    if (path.empty()) return "/";
    
    // Truncate if too long
    std::string sanitized = path.substr(0, MAX_PATH_LENGTH);
    
    // Remove dangerous patterns
    sanitized.erase(
        std::remove_if(sanitized.begin(), sanitized.end(), 
            [](char c) { return c == '\0' || c == '\r' || c == '\n'; }),
        sanitized.end()
    );
    
    // Block directory traversal
    if (sanitized.find("..") != std::string::npos) {
        return "/"; // Return root if traversal detected
    }
    
    // Block null bytes and other dangerous chars
    if (sanitized.find('\0') != std::string::npos) {
        return "/";
    }
    
    // Ensure it starts with /
    if (sanitized.empty() || sanitized[0] != '/') {
        sanitized = "/" + sanitized;
    }
    
    return sanitized;
}

// Validate buffer size to prevent integer overflow
bool Security::is_safe_buffer_size(size_t size, size_t max_size) {
    return size > 0 && size <= max_size;
}

