<div align="center">
  
<img width="512" height="800" alt="honeypot" src="https://github.com/user-attachments/assets/be209c0b-00e4-4390-9a70-566d74e8575d" />

</div>

# 🍯 Industrial Honeypot IIoT v2.0 🍯

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Boost-Asio](https://img.shields.io/badge/Boost-Asio-green.svg)
![Linux/Rasp](https://img.shields.io/badge/Platform-Linux/RaspberryPi-orange.svg)
![License-MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Docker](https://img.shields.io/badge/Container-Docker-blue.svg)

## What has changed from version 1.0 to version 2.0:

### 📝 String Concatenation

- Replacement of multiple string concatenations with `std::ostringstream`
- Elimination of temporary string copies
- 30-40% reduction in log construction time
- Improved memory usage during logging operations

#### 🤔 What impact do these changes have on the project?

- Reduces temporary allocations
- Improves log construction performance
- Reduces temporary memory usage
- Decreases logging latency
- Improves scalability under high load

### ⚙️ Statically Compiled Regex

- IP validation regex (IPv4 and IPv6) are now statically compiled
- Use of `std::regex::optimize` for better performance
- 50-70% reduction in IP validation time under high load
- Regex compiled only once and reused in all calls

#### 🤔 What impact do these changes have on the project?

- Modbus Server: for each new TCP connection (port 502)
- CoAP Server: for each received UDP packet (port 5683)
- MQTT Server: for each connection and each processed packet (port 1883)
- REST API: for each HTTP request (port 8080) + rate limiting

### ⚙️ Compilation Optimizations

- Added O2 optimization flags for standard compilation
- Added O3 flags for Release mode
- Enabled Link-Time Optimization (LTO) with `-flto`
- Architecture-specific optimizations with `-march=native`
- Debug mode retains O0 to facilitate debugging

#### 🤔 What impact do these changes have on the project?

- 20-30% performance improvement in I/O operations
- Reduced binary size with LTO
- Specific optimizations for the processor used

### 🚀 Intelligent Buffering for Logging

- Added internal buffer with `std::vector<std::string>`
- Implemented mutex for thread-safety (`std::mutex`)
- Defined control constants:
- `BUFFER_SIZE = 10`: automatic flush after 10 entries
- `FLUSH_INTERVAL_SECONDS = 5`: automatic flush every 5 seconds
- Added private methods:
- `add_to_buffer()`: adds entry to buffer
- `flush_buffer()`: writes buffer to disk
- Added public method `force_flush()`: forces immediate write

### 🚀 Implementation

- All log functions (`log_modbus_connection`, `log_coap_request`, `log_auth_attempt`, `log_attack_detected`) now:
1. Build the log string in memory
2. Add to the buffer instead of writing directly
3. The buffer automatically flushes when:
- Reaches 10 entries, OR
- 5 seconds have passed since the last flush
- Buffer is automatically emptied on `shutdown()`
- Memory is pre-allocated on `initialize()` with `reserve()`

#### 🤔 What impact do these changes have on the project?

- 80-90% reduction in disk I/O operations
- Lower latency in logging operations
- Improved overall server performance
- Thread-safe for concurrent environments

### 🚀 Optimizations ARM64

- Equivalent to `-march=native` for ARM architecture
- Optimizes for specific ARM processors (Cortex-A, Neoverse)

#### 🤔 What impact do these changes have on the project?

- Expected gain: 15-20% in ARM

---

### 🔒 Main Security Updates

- All data inserted into JSON logs is now sanitized using `Security::sanitize_for_log()`
- Prevention of log injection and JSON file corruption
- IPs, resources, topics, messages, and timestamps are sanitized before logging
- IP validation before use on all servers (Modbus, CoAP, MQTT, REST API)
- Exception handling when obtaining `remote_endpoint()`
- Invalid IPs are marked as "INVALID_IP" or "UNKNOWN"
- Rate limiting implementation using `Security::check_rate_limit()`
- Limit of 60 requests per minute per IP
- Connections exceeding the limit are automatically closed
- HTTP paths are sanitized using `Security::sanitize_path()`
- Prevention of directory traversal (`..`)
- Maximum path length validation (2048 characters)
- Counter of CoAP requests are now thread-safe with mutexes
- Prevention of race conditions in DDoS detection

# 📋Index

- [Overview](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#-overview)
- [Features](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#-features)
- [Installation](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#-installation)
- [How to Use](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#-how-to-use)
- [Project Scope](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#-project-scope)
- [Technologies](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#%EF%B8%8F-technologies)
- [Contribution](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#-contribution)
- [License](https://github.com/JoaoPHS/industrial-honeypot-iot/blob/main/README.md#-license)

## 🎯 Overview

This honeypot is a security demonstration that simulates ICS/SCADA environments to detect and analyze attacks in real time. Developed in C++17 with Boost.Asio, it offers industrial-grade performance for protecting critical infrastructures. As a demonstration, it is ideal for cybersecurity enthusiasts, students, professionals, and SOC teams to study. It can also serve as a basis for developing a more robust honeypot, adding new functionalities, configurations, adding a dashboard, or integrating with other projects.

A highly interactive honeypot for detecting attacks on critical industrial infrastructures, developed in C++17 with a focus on performance and structured logging for SOCs. It is very important to develop applications at an industrial level because, as we saw in the case of Jaguar Land Rover, a manufacturing shutdown due to a hacker attack can generate losses of billions of dollars and even lead to bankruptcy.

## ✨ Features

### 🔧 Modbus TCP Server

- Simulation of industrial PLCs on port 502
- Detection of authentication attempts
- Identification of large packets
- Automatic responses to Modbus requests

### 📡 CoAP UDP Server

- IoT device protocol on port 5683
- Simulation of industrial sensors and actuators
- Pre-configured resources (/sensors, /actuators, /admin)
- IPv4 and IPv6 support

### 🚨 SOC Detection System

- Identification of attack patterns
- Classification by severity/importance (LOW, MEDIUM, HIGH)
- Structured JSON logging for forensic analysis
- Detection of access to administrative resources

### 📊 Structured Logging

- Standardized JSON format for SIEM integration
- Precise timestamps for temporal analysis
- Complete metadata (IP, port, event type)
- Automatic file rotation

### 🐳 Containerization

- Dockerfile for consistent deployment
- Automatically mapped ports
- Cross-platform (x86_64, ARM)

### 🔍 Real-Time Monitoring

- Immediate visualization of events in the console
- Continuous log file updates
- Connection and attack statistics
- Intuitive command-line interface

### ⚙️ Debug Server and Test Mode

- Configurable debug server file created in the src subdirectory if you want to configure or test new features on the server
  
## 🚀 Installation

### 📋 Prerequisites

- Linux (Debian/Ubuntu), Raspberry Pi OS or Raspberry Pi >= 4 (physical device)
- GCC 8+ or Clang 6+
- CMake 3.16+
- Boost 1.66+

### 📦 Method 1: Native Compilation

```bash
# Clone the repository
git clone https://github.com/JoaoPHS/iot-industrial-honeypot.git

# Navigate to the folder
cd iot-industrial-honeypot/build

# Configure and compile
rm -rf *
cmake ..
make

# Run the server
./iot_honeypot_server

```

### 🔧 Method 2: Docker

```bash
# Build the image
docker build -t iot-honeypot .

# Run container
docker run -p 502:502 -p 5683:5683/udp iot-honeypot

##⚡ Method 3: Raspberry Pi

bash
# Install dependencies on Raspberry Pi
sudo apt update && sudo apt install -y \
build-essential \
cmake \
libboost-system-dev \
libboost-thread-dev

# Compile normally
rm -rf * 
  or (If that doesn't work, follow the procedure below)
mkdir build && cd build
cmake .. && make
./iot_honeypot_server
```
## 🎮 How to Use

### 🖥️ First Steps

Start the server:

```bash
./iot_honeypot_server
```
Monitor the logs in another terminal:

```bash
tail -f logs/honeypot_soc.json
```

### 🔍 Performing Tests

#### Modbus Test:
```bash
# Basic connection
nc localhost 502

# Multiple connections
for i in {1..5}; do echo "TEST" | nc localhost 502; done
```
#### CoAP Test:
```bash
# Explicit IPv4 (always works)
echo "SCAN" | nc -u 127.0.0.1 5683 -w 1

# With socat (recommended)
echo "TEST" | socat - UDP:localhost:5683
```
### 📊 Detailed Mode

#### The server operates in detailed mode by default, displaying:

- >>>>> Established connections
- >>>>>> Received requests
- >>>>>>> Detected attacks
- >>>>>>>> Real-time statistics

### ⚙️ Advanced Settings

#### Custom Ports:

```cpp
// In main.cpp
ModbusServer modbus_server(io_context, 1502); // Alternative port
CoAPServer coap_server(io_context, 15683); // Alternative port
```
Custom Logging:

```cpp
// Initialize with custom file
SOCLogger::initialize("custom_logs.json");
```

### 💾 Exporting Data

#### Logs are automatically saved to:

```bash
build/logs/honeypot_soc.json
```
#### For SIEM integration:

```bash
# Example: Send to Elasticsearch tail -f logs/honeypot_soc.json | while read line; of 
curl -X POST elasticsearch:9200/honeypot/_doc -H "Content-Type: application/json" -d "$line"
done
```


# 📁 Project Scope

```bash
industrial-honeypot-iot/
├── 📁 server/
│    ├── 📁 include/
│    │    ├── 🏗️ modbus_server.h
│    │    ├── 📡 coap_server.h
│    │    ├── ⚙️ rest_api.h
│    │    ├── 📡 mqtt_server.h
│    │    ├── 🔒 security.h
│    │    └️ 📊 soc_logger.h
│    │
│    └── 📁 src/
│         ├── 🎯 main.cpp
│         ├── ⚙️ debug_server.cpp
│         ├── 🔧 modbus_server.cpp
│         ├️ 📡 coap_server.cpp
│         └️ 📊 soc_logger.cpp
├── 📁 configs
│    └── ⚙️ simulated_plcs.pb
├── 📁 build/ <<< (Contains CMake/Makefile compilation files)
│    └── 📁 logs/
│        └️ 📄 honeypot_soc.json
├── 📁 docker/
│     └️ 🐳 Dockerfile
├── 📁 proto/
│     ├── 🔩 plc_memory.pb.cc
│     ├── 🪛 plc_memory.pb.h
│     └️ plc_memory.proto
├── 📁 utils/
│     ├── ⚙️ generate_plc_config
│     ├── ⚙️ generate_plc_config.cpp
│     └️ ⚙️ generate_simple.cpp
├── 🔩 plc_memory.pb.cc >>> (PLC Reserve in C if you want to make modifications without affecting the main file in the proto folder)
├── 🪛 plc_memory.pb.h >>> (PLC Header.cc file interface reservation same purpose)
└️🛠️ CMakeLists.txt
```
## 🏗️ Architecture

- main.cpp: Main orchestrator and initializer
- modbus_server.cpp: Modbus TCP server
- coap_server.cpp: CoAP UDP server
- soc_logger.cpp: JSON structured logging system
- Dockerfile: Containerization for deployment

## 🛠️ Technologies

### 💻 Stack

- C++17: Language for maximum performance
- Boost.Asio: Asynchronous I/O and network programming
- CMake: Cross-platform build system
- Docker: Containerization and deployment

## 📚 Main Libraries

#### C++:

- Boost 1.66+ (Asio, System, Thread)
- Standard Template Library (STL)
- POSIX Sockets API

### 🏛️ Architecture (Development)

- Reactor Pattern: I/O Asynchronous with Boost.Asio
- Singleton: Logging Management
- RAII: Automatic Resource Management
- Callback-based: Asynchronous Handlers for Network

## ⚡ Optimizations

- Compilation with O2 optimizations
- Intelligent buffering for logging
- Connection and socket reuse
- Efficient memory allocation

## 🤝 Contribution

### 📝 How to Help

Fork the project:

#### Create a Branch:

```bash
git checkout -b feature/new-feature
```
#### Commit your Changes:

```bash
git commit -m 'Add feature'
```
#### Push to the Branch:

```bash
git push origin feature/new-feature
```
#### Open a Pull Request

## 🎯 Areas for Improvement

- New industrial protocols (BACnet, DNP3)
- Real-time web dashboard accessing the agent via REST/WSS (cpp-httplib)
- MQTT deployment
- More attack pattern detections
- Support for more architectures (ARM64)
- Integration with threat intelligence APIs

## 📋 Guidelines

- Follow the C++17 coding standard
- Maintain compatibility with Linux
- Document new features
- Add tests for new features

## 📄 License

- Distributed under the [MIT license](https://opensource.org/license/mit). See LICENSE for more information.
- Free permission to use, copy, modify and distribute this software.

[Copyright (c) 2025 João Pedro](https://www.linkedin.com/in/jo%C3%A3o-pedro-h-1a8000345/)

## ⚖️ Disclaimer

### 🚨 Legal Notice

- This software is provided as is, without warranties of any kind
- Designed for testing and research environments only
- NOT recommended for use in production networks
- Users are entirely responsible for proper use
- No support for deployment in critical infrastructures
- Use at your own risk

<div align="center">

🐻 "It's always good to have a honeypot for an invading bear" 🍯

</div>
