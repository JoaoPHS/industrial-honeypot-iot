#include "security.h"
#include <regex>
#include <sstream>
#include <cctype>
#include <iomanip>

std::map<std::string, std::chrono::steady_clock::time_point> Security::rate_limit_map_;
std::map<std::string, int> Security::request_count_map_;
std::mutex Security::rate_limit_mutex_;

std::string Security::sanitize_for_log(const std::string& input) {
    if (input.empty()) {
        return "";
    }
    
    std::string sanitized;
    sanitized.reserve(input.length());
    
    for (char c : input) {
        if (std::isprint(c) && c != '\n' && c != '\r' && c != '\t') {
            if (c == '"' || c == '\\') {
                sanitized += '\\';
                sanitized += c;
            } else {
                sanitized += c;
            }
        } else if (c == '\n') {
            sanitized += "\\n";
        } else if (c == '\r') {
            sanitized += "\\r";
        } else if (c == '\t') {
            sanitized += "\\t";
        } else {
            std::ostringstream oss;
            oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') 
                << (static_cast<unsigned char>(c) & 0xFF);
            sanitized += oss.str();
        }
    }
    
    if (sanitized.length() > MAX_LOG_STRING_LENGTH) {
        sanitized = sanitized.substr(0, MAX_LOG_STRING_LENGTH - 3);
        sanitized += "...";
    }
    
    return sanitized;
}

bool Security::is_valid_ip(const std::string& ip) {
    if (ip.empty() || ip.length() > 45) {
        return false;
    }
    
    static const std::regex ipv4_pattern(R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)", std::regex::optimize);
    static const std::regex ipv6_pattern(R"(^(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]+|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$)", std::regex::optimize);
    
    if (std::regex_match(ip, ipv4_pattern)) {
        return true;
    }
    
    if (std::regex_match(ip, ipv6_pattern)) {
        return true;
    }
    
    return false;
}

bool Security::check_rate_limit(const std::string& ip, int max_requests_per_minute) {
    if (ip.empty() || max_requests_per_minute <= 0) {
        return false;
    }
    
    if (!is_valid_ip(ip)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(rate_limit_mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto it = rate_limit_map_.find(ip);
    
    if (it == rate_limit_map_.end()) {
        rate_limit_map_[ip] = now;
        request_count_map_[ip] = 1;
        return true;
    }
    
    auto time_diff = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
    
    if (time_diff >= 60) {
        rate_limit_map_[ip] = now;
        request_count_map_[ip] = 1;
        return true;
    }
    
    int& count = request_count_map_[ip];
    
    if (count >= max_requests_per_minute) {
        return false;
    }
    
    count++;
    return true;
}

std::string Security::sanitize_path(const std::string& path) {
    if (path.empty()) {
        return "/";
    }
    
    std::string sanitized;
    sanitized.reserve(path.length());
    
    bool last_was_slash = false;
    bool found_dot_dot = false;
    int dot_count = 0;
    
    for (size_t i = 0; i < path.length() && sanitized.length() < MAX_PATH_LENGTH; ++i) {
        char c = path[i];
        
        if (c == '/') {
            if (!last_was_slash) {
                sanitized += '/';
                last_was_slash = true;
                dot_count = 0;
                found_dot_dot = false;
            }
        } else if (c == '.') {
            dot_count++;
            if (dot_count >= 2) {
                found_dot_dot = true;
            }
            sanitized += c;
            last_was_slash = false;
        } else if (std::isalnum(c) || c == '-' || c == '_' || c == '~' || c == ':' || c == '@' || c == '&' || c == '=' || c == '+' || c == '$' || c == ',' || c == ';') {
            sanitized += c;
            last_was_slash = false;
            dot_count = 0;
            found_dot_dot = false;
        } else if (c == '%') {
            if (i + 2 < path.length()) {
                char hex1 = path[i + 1];
                char hex2 = path[i + 2];
                if (std::isxdigit(hex1) && std::isxdigit(hex2)) {
                    sanitized += c;
                    sanitized += hex1;
                    sanitized += hex2;
                    i += 2;
                    last_was_slash = false;
                    dot_count = 0;
                    found_dot_dot = false;
                    continue;
                }
            }
        } else {
            sanitized += '_';
            last_was_slash = false;
            dot_count = 0;
        }
    }
    
    if (found_dot_dot) {
        size_t pos = sanitized.find("..");
        while (pos != std::string::npos) {
            sanitized.replace(pos, 2, "__");
            pos = sanitized.find("..", pos + 2);
        }
    }
    
    if (sanitized.empty() || sanitized[0] != '/') {
        sanitized = "/" + sanitized;
    }
    
    if (sanitized.length() > MAX_PATH_LENGTH) {
        sanitized = sanitized.substr(0, MAX_PATH_LENGTH);
    }
    
    while (sanitized.length() > 1 && sanitized.back() == '/') {
        sanitized.pop_back();
    }
    
    return sanitized;
}

bool Security::is_safe_buffer_size(size_t size, size_t max_size) {
    if (max_size == 0) {
        return false;
    }
    
    if (size > max_size) {
        return false;
    }
    
    constexpr size_t MAX_SAFE_BUFFER = 10 * 1024 * 1024;
    
    if (size > MAX_SAFE_BUFFER) {
        return false;
    }
    
    if (max_size > MAX_SAFE_BUFFER) {
        return false;
    }
    
    return true;
}

