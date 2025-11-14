cmake_minimum_required(VERSION 3.16)
project(iot_honeypot_soc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/server/include
)

# ✅ Add soc_loger.cpp to the list of sources
set(SERVER_SOURCES
    server/src/main.cpp
    server/src/modbus_server.cpp
    server/src/coap_server.cpp
    server/src/soc_logger.cpp  
)

add_executable(iot_honeypot_server ${SERVER_SOURCES})

target_link_libraries(iot_honeypot_server 
    PRIVATE 
    ${Boost_LIBRARIES}
    pthread
)

target_include_directories(iot_honeypot_server PRIVATE ${Boost_INCLUDE_DIRS})