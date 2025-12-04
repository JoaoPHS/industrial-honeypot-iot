#include <iostream>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include "modbus_server.h"
#include "coap_server.h"
#include "mqtt_server.h"
#include "rest_api.h"
#include "soc_logger.h"

int main() {
    std::cout << "Industrial Honeypot IoT - Enhanced v2.0" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        // Initializes the SOCLogger
        SOCLogger::initialize("honeypot_soc.json");
        
        boost::asio::io_context io_context;
        
        // Start all servers
        ModbusServer modbus_server(io_context, 502);
        CoAPServer coap_server(io_context, 5683);
        MQTTServer mqtt_server(io_context, 1883);
        RestAPI rest_api(io_context, 8080);

        std::cout << "=====================================" << std::endl;
        std::cout << "\nServers Started:" << std::endl;
        std::cout << "   - TCP Modbus: Port 502" << std::endl;
        std::cout << "   - UDP CoAP: Port 5683" << std::endl;
        std::cout << "   - TCP MQTT: Port 1883" << std::endl;
        std::cout << "   - HTTP REST API: Port 8080" << std::endl;
        std::cout << "\nDashboard: http://localhost:8080" << std::endl;
        std::cout << "API Stats: http://localhost:8080/api/stats" << std::endl;
        std::cout << "\nWaiting for Connections..." << std::endl;
        std::cout << "Use Ctrl+C to Finish" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        // Run servers
        std::thread io_thread([&io_context]() {
            io_context.run();
        });
        
        // Wait indefinitely (or Ctrl+C)
        io_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        SOCLogger::shutdown();
        return 1;
    }
    
    // End logger
    // It usually doesn't get this far but it's a precaution
    SOCLogger::shutdown();
    return 0;
}