# Industrial Honeypot IIoT v1.0 

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Boost-Asio](https://img.shields.io/badge/Boost-Asio-green.svg)
![Linux/Rasp](https://img.shields.io/badge/Platform-Linux/RaspberryPi-orange.svg)
![License-MIT](https://img.shields.io/badge/License-MIT-yellow.svg)


📋Index

- [Overview](#overview)
- [Features](#features)
- [Installation](#installation)
- [How to Use](#how-to-use)
- [Project Scope](#project-scope)
- [Technologies](#technologies)
- [Contribution](#contribution)
- [License](#license)

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
- Test directory created if you want to add new features and test them in isolation

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

# UNDER CONSTRUCTION...
