# IIoT Industrial Honeypot v1.0 

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

# UNDER CONSTRUCTION...
