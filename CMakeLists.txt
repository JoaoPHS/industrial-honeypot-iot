cmake_minimum_required(VERSION 3.16)
project(iot_honeypot_soc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64")
    set(IS_ARM64 TRUE)
    message(STATUS "Detected ARM64 architecture")
else()
    set(IS_ARM64 FALSE)
    message(STATUS "Detected x86_64 architecture")
endif()

# Optimization flags (architecture-aware)
if(IS_ARM64)
    # ARM64-specific optimizations
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -mcpu=native -flto")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -mcpu=native -flto")
else()
    # x86_64 optimizations
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native -flto")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

find_package(Boost REQUIRED COMPONENTS system)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/server/include
)

# Add all server sources
set(SERVER_SOURCES
    server/src/main.cpp
    server/src/modbus_server.cpp
    server/src/coap_server.cpp
    server/src/mqtt_server.cpp
    server/src/rest_api.cpp
    server/src/soc_logger.cpp  
)

add_executable(iot_honeypot_server ${SERVER_SOURCES})

target_link_libraries(iot_honeypot_server 
    PRIVATE 
    ${Boost_LIBRARIES}
    pthread
)

target_include_directories(iot_honeypot_server PRIVATE ${Boost_INCLUDE_DIRS})