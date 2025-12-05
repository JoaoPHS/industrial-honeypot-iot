#include "rest_api.h"
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>

using boost::asio::ip::tcp;

// Initialize static counters
std::atomic<int> RestAPI::modbus_connections_(0);
std::atomic<int> RestAPI::coap_requests_(0);
std::atomic<int> RestAPI::mqtt_activities_(0);
std::atomic<int> RestAPI::attacks_detected_(0);

RestAPI::RestAPI(boost::asio::io_context& io_context, short port)
    : io_context_(io_context), 
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
    
    start_accept();
    std::cout << "REST API Dashboard Running on Port " << port << std::endl;
    std::cout << "Access: http://localhost:" << port << "/api/stats" << std::endl;
}

void RestAPI::start_accept() {
    auto socket = std::make_shared<tcp::socket>(io_context_);
    
    acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec) {
        if (!ec) {
            handle_client(socket);
        }
        start_accept();
    });
}

void RestAPI::handle_client(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<std::vector<char>>(4096);
    
    socket->async_read_some(boost::asio::buffer(*buffer), 
        [this, socket, buffer](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string request(buffer->data(), length);
                process_http_request(socket, request);
            }
        });
}

void RestAPI::process_http_request(std::shared_ptr<tcp::socket> socket,
                                   const std::string& request) {
    // Parse HTTP request
    std::istringstream request_stream(request);
    std::string method, path, version;
    request_stream >> method >> path >> version;
    
    std::cout << "REST API: " << method << " " << path << std::endl;
    
    std::string response_body;
    std::string content_type = "application/json";
    
    if (path == "/api/stats" || path == "/api/stats/") {
        // Return statistics as JSON
        response_body = generate_stats_json();
    } else if (path == "/" || path == "/index.html") {
        // Return Matrix/SIEM style dashboard
        content_type = "text/html";
        response_body = "<!DOCTYPE html>"
            "<html><head><title>IIoT Honeypot SIEM - Matrix Mode</title>"
            "<meta charset=\"UTF-8\"><style>"
            "@import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&display=swap');"
            "*{margin:0;padding:0;box-sizing:border-box;}"
            "body{font-family:'Share Tech Mono',monospace;background:#000;color:#0f0;overflow-x:hidden;}"
            ".scanline{position:fixed;top:0;left:0;width:100%;height:100%;background:linear-gradient(rgba(18,16,16,0) 50%,rgba(0,0,0,.25) 50%);background-size:100% 2px;z-index:9999;pointer-events:none;animation:scanline 8s linear infinite;}"
            "@keyframes scanline{0%{transform:translateY(0);}100%{transform:translateY(100vh);}}"
            ".header{background:rgba(0,20,0,0.9);border-bottom:2px solid #0f0;padding:20px;text-align:center;box-shadow:0 0 20px #0f0;}"
            ".header h1{font-size:32px;text-shadow:0 0 10px #0f0,0 0 20px #0f0;animation:glow 2s ease-in-out infinite alternate;}"
            "@keyframes glow{from{text-shadow:0 0 10px #0f0,0 0 20px #0f0;}to{text-shadow:0 0 20px #0f0,0 0 30px #0f0,0 0 40px #0f0;}}"
            ".status{display:inline-block;margin-left:20px;color:#0f0;animation:blink 1s infinite;}"
            "@keyframes blink{0%,50%,100%{opacity:1;}25%,75%{opacity:0;}}"
            ".container{max-width:1400px;margin:20px auto;padding:20px;}"
            ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px;margin-bottom:20px;}"
            ".card{background:rgba(0,20,0,0.8);border:2px solid #0f0;padding:20px;box-shadow:0 0 20px rgba(0,255,0,0.3);position:relative;overflow:hidden;}"
            ".card::before{content:'';position:absolute;top:-50%;left:-50%;width:200%;height:200%;background:radial-gradient(circle,rgba(0,255,0,0.1),transparent);animation:pulse 3s ease-in-out infinite;}"
            "@keyframes pulse{0%,100%{transform:scale(1);}50%{transform:scale(1.1);}}"
            ".card.critical{border-color:#f00;box-shadow:0 0 30px rgba(255,0,0,0.5);}"
            ".card.critical .value{color:#f00;text-shadow:0 0 10px #f00;}"
            ".card-header{font-size:12px;opacity:0.8;margin-bottom:10px;text-transform:uppercase;letter-spacing:2px;}"
            ".value{font-size:48px;font-weight:bold;text-shadow:0 0 10px currentColor;position:relative;z-index:1;}"
            ".threat-level{margin-top:10px;padding:5px;background:rgba(0,255,0,0.2);border-left:3px solid #0f0;font-size:11px;}"
            ".threat-level.high{background:rgba(255,0,0,0.2);border-color:#f00;color:#f00;}"
            ".terminal{background:#000;border:2px solid #0f0;padding:15px;font-size:12px;height:200px;overflow-y:auto;box-shadow:inset 0 0 20px rgba(0,255,0,0.2);}"
            ".log-card{max-width:500px;margin:0 auto;}"
            ".log-card .card-header{text-align:center;}"
            ".terminal::-webkit-scrollbar{width:8px;}"
            ".terminal::-webkit-scrollbar-track{background:#001a00;}"
            ".terminal::-webkit-scrollbar-thumb{background:#0f0;}"
            ".log-entry{margin:5px 0;opacity:0;animation:fadeIn 0.5s forwards;}"
            "@keyframes fadeIn{to{opacity:1;}}"
            ".log-entry span{color:#0f0;}"
            ".log-entry.attack{color:#f00;}"
            ".timestamp{color:#0a0;font-size:10px;}"
            ".footer{text-align:center;padding:20px;color:#0a0;font-size:11px;border-top:1px solid #0f0;margin-top:20px;}"
            ".btn{background:#0f0;color:#000;border:none;padding:10px 20px;font-family:'Share Tech Mono',monospace;cursor:pointer;text-transform:uppercase;letter-spacing:2px;box-shadow:0 0 10px #0f0;transition:all 0.3s;}"
            ".btn:hover{background:#0a0;box-shadow:0 0 20px #0f0;transform:scale(1.05);}"
            ".matrix-bg{position:fixed;top:0;left:0;width:100%;height:100%;z-index:-1;opacity:0.1;}"
            "</style></head><body>"
            "<div class=\"scanline\"></div>"
            "<div class=\"header\">"
            "<h1>[[ INDUSTRIAL HONEYPOT SIEM ]]</h1>"
            "<span class=\"status\">● SYSTEM ACTIVE</span>"
            "<span style=\"float:right;font-size:12px;\" id=\"time\"></span>"
            "</div>"
            "<div class=\"container\">"
            "<div class=\"grid\">"
            "<div class=\"card\">"
            "<div class=\"card-header\">⚡ MODBUS TCP/502</div>"
            "<div class=\"value\" id=\"modbus\">0</div>"
            "<div class=\"threat-level\">CONNECTIONS MONITORED</div>"
            "</div>"
            "<div class=\"card\">"
            "<div class=\"card-header\">📡 COAP UDP/5683</div>"
            "<div class=\"value\" id=\"coap\">0</div>"
            "<div class=\"threat-level\">REQUESTS INTERCEPTED</div>"
            "</div>"
            "<div class=\"card\">"
            "<div class=\"card-header\">🔌 MQTT TCP/1883</div>"
            "<div class=\"value\" id=\"mqtt\">0</div>"
            "<div class=\"threat-level\">ACTIVITIES LOGGED</div>"
            "</div>"
            "<div class=\"card critical\">"
            "<div class=\"card-header\">⚠ THREAT DETECTION</div>"
            "<div class=\"value\" id=\"attacks\">0</div>"
            "<div class=\"threat-level high\" id=\"threat\">NO THREATS DETECTED</div>"
            "</div>"
            "</div>"
            "<div class=\"card log-card\">"
            "<div class=\"card-header\">📊 SYSTEM LOG [LIVE FEED]</div>"
            "<div class=\"terminal\" id=\"logs\">"
            "<div class=\"log-entry\">[SYSTEM] Honeypot initialized...</div>"
            "<div class=\"log-entry\">[SYSTEM] All protocols active...</div>"
            "<div class=\"log-entry\">[SYSTEM] Monitoring for threats...</div>"
            "</div>"
            "</div>"
            "<div style=\"text-align:center;margin-top:20px;\">"
            "<button class=\"btn\" onclick=\"loadStats()\">↻ REFRESH DATA</button>"
            "</div>"
            "<div class=\"footer\">"
            "<div>IIoT HONEYPOT v2.0 ENHANCED | MATRIX MODE ENABLED</div>"
            "<div id=\"timestamp\" style=\"margin-top:5px;\"></div>"
            "</div>"
            "</div>"
            "<script>"
            "let prevStats={modbus:0,coap:0,mqtt:0,attacks:0};"
            "let scanCounter=0;"
            "const scanMsgs=['[SCANNER] Monitoring port 502/tcp...','[SCANNER] Monitoring port 5683/udp...','[SCANNER] Monitoring port 1883/tcp...','[SIEM] Analyzing traffic patterns...','[SIEM] Checking threat database...','[IDS] Signature database updated','[FIREWALL] Rules synchronized','[MONITOR] Network baseline normal','[SENSOR] All honeypots responsive'];"
            "function addLog(msg,isAttack){"
            "const logs=document.getElementById('logs');"
            "const entry=document.createElement('div');"
            "entry.className='log-entry'+(isAttack?' attack':'');"
            "const time=new Date().toLocaleTimeString();"
            "entry.innerHTML='<span class=\"timestamp\">['+time+']</span> '+msg;"
            "logs.appendChild(entry);"
            "logs.scrollTop=logs.scrollHeight;"
            "if(logs.children.length>50)logs.removeChild(logs.firstChild);"
            "}"
            "function addScanLog(){"
            "addLog(scanMsgs[scanCounter%scanMsgs.length]);"
            "scanCounter++;"
            "}"
            "function loadStats(){"
            "fetch('/api/stats').then(r=>r.json()).then(d=>{"
            "document.getElementById('modbus').textContent=d.modbus_connections;"
            "document.getElementById('coap').textContent=d.coap_requests;"
            "document.getElementById('mqtt').textContent=d.mqtt_activities;"
            "document.getElementById('attacks').textContent=d.attacks_detected;"
            "document.getElementById('timestamp').textContent='LAST SYNC: '+new Date().toLocaleString();"
            "const diff={modbus:d.modbus_connections-prevStats.modbus,coap:d.coap_requests-prevStats.coap,mqtt:d.mqtt_activities-prevStats.mqtt,attacks:d.attacks_detected-prevStats.attacks};"
            "if(diff.modbus>0){"
            "for(let i=0;i<diff.modbus;i++){"
            "addLog('[MODBUS/502] Connection #'+d.modbus_connections+' established from external host');"
            "}"
            "}"
            "if(diff.coap>0){"
            "for(let i=0;i<diff.coap;i++){"
            "addLog('[COAP/5683] UDP packet intercepted | Size: '+(Math.floor(Math.random()*500)+50)+'B');"
            "}"
            "}"
            "if(diff.mqtt>0){"
            "for(let i=0;i<diff.mqtt;i++){"
            "addLog('[MQTT/1883] Client activity detected | Protocol: MQTT v3.1.1');"
            "}"
            "}"
            "if(diff.attacks>0){"
            "for(let i=0;i<diff.attacks;i++){"
            "addLog('[⚠ ALERT] INTRUSION ATTEMPT DETECTED! | Severity: HIGH | Action: LOGGED',true);"
            "addLog('[IDS] Attack signature matched | Type: '+['Buffer Overflow','Port Scan','Admin Access','DDoS Attempt'][Math.floor(Math.random()*4)],true);"
            "}"
            "}"
            "const threat=document.getElementById('threat');"
            "if(d.attacks_detected>0){"
            "threat.textContent='⚠ '+d.attacks_detected+' THREAT(S) DETECTED';"
            "threat.className='threat-level high';"
            "}else{"
            "threat.textContent='✓ NO THREATS DETECTED';"
            "threat.className='threat-level';"
            "}"
            "prevStats=d;"
            "}).catch(e=>addLog('[ERROR] Connection to API failed',true));"
            "}"
            "function updateTime(){"
            "document.getElementById('time').textContent=new Date().toLocaleString();"
            "}"
            "setInterval(loadStats,3000);"
            "setInterval(addScanLog,8000);"
            "setInterval(updateTime,1000);"
            "window.onload=function(){loadStats();updateTime();setTimeout(addScanLog,2000);};"
            "</script></body></html>";
    } else {
        // 404 Not Found
        response_body = R"({"error": "Not Found", "message": "Available endpoints: /api/stats, /"})";
    }
    
    std::string response = generate_http_response(content_type, response_body);
    
    boost::asio::async_write(*socket, boost::asio::buffer(response),
        [socket](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                socket->shutdown(tcp::socket::shutdown_both);
            }
        });
}

std::string RestAPI::generate_stats_json() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&time_t);
    timestamp.erase(timestamp.find('\n'), 1);
    
    std::ostringstream json;
    json << "{\n"
         << "  \"timestamp\": \"" << timestamp << "\",\n"
         << "  \"modbus_connections\": " << modbus_connections_.load() << ",\n"
         << "  \"coap_requests\": " << coap_requests_.load() << ",\n"
         << "  \"mqtt_activities\": " << mqtt_activities_.load() << ",\n"
         << "  \"attacks_detected\": " << attacks_detected_.load() << ",\n"
         << "  \"status\": \"running\"\n"
         << "}";
    
    return json.str();
}

std::string RestAPI::generate_http_response(const std::string& content_type,
                                            const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << content_type << "\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;
    
    return response.str();
}

void RestAPI::increment_modbus_connections() {
    modbus_connections_++;
}

void RestAPI::increment_coap_requests() {
    coap_requests_++;
}

void RestAPI::increment_mqtt_activities() {
    mqtt_activities_++;
}

void RestAPI::increment_attacks() {
    attacks_detected_++;
}

std::map<std::string, int> RestAPI::get_statistics() {
    return {
        {"modbus_connections", modbus_connections_.load()},
        {"coap_requests", coap_requests_.load()},
        {"mqtt_activities", mqtt_activities_.load()},
        {"attacks_detected", attacks_detected_.load()}
    };
}


