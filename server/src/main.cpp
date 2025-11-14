#include <iostream>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include "modbus_server.h"
#include "coap_server.h"  
#include "soc_logger.h"

int main() {
    std::cout << "🎯 Industrial Honeypot IoT - Demo" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        // ✅ Initializes the SOCLogger
        SOCLogger::initialize("honeypot_soc.json");
        
        boost::asio::io_context io_context;
        
        // ✅ Start existing servers
        ModbusServer modbus_server(io_context, 502);
        CoAPServer coap_server(io_context, 5683);

        std::cout << "=====================================" << std::endl;
        std::cout << "\n📡 Servers Started:" << std::endl;
        std::cout << "   - TCP Modbus: Port > 502" << std::endl;
        std::cout << "   - UDP CoAP: Port > 5683" << std::endl;
        std::cout << "\n🔍 Waiting Connections..." << std::endl;
        std::cout << "💡 Use Ctrl+C to Finish" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        // ✅ Run servers
        std::thread io_thread([&io_context]() {
            io_context.run();
        });
        
        // Wait indefinitely (or Ctrl+C)
        io_thread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "💥 Error: " << e.what() << std::endl;
        SOCLogger::shutdown();
        return 1;
    }
    
    /* ✅ End logger 
     - It usually doesn't get this far. 
     - But if all else fails, disable the SOCLogger. 
     - It's a precaution
     */
    SOCLogger::shutdown();
    return 0;
}