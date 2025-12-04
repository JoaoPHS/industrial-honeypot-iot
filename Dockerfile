# Dockerfile for x86_64 (Windows/Linux/Mac Intel)
# Multi-stage build for smaller final image

FROM ubuntu:22.04 AS builder

# Prevent interactive prompts during build
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libboost-system-dev \
    libboost-thread-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Copy project files
COPY . .

# Clean any existing build directory and compile
RUN rm -rf build && \
    mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Final stage - runtime only
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install only runtime dependencies
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    && rm -rf /var/lib/apt/lists/*

# Create directory for logs
RUN mkdir -p /app/logs && chmod 777 /app/logs

WORKDIR /app

# Copy compiled binary from builder
COPY --from=builder /workspace/build/iot_honeypot_server .

# Expose all ports
# Modbus TCP
EXPOSE 502/tcp
# CoAP UDP
EXPOSE 5683/udp
# MQTT TCP
EXPOSE 1883/tcp
# REST API / Dashboard
EXPOSE 8080/tcp

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD netstat -an | grep -E '(:502|:1883|:8080)' > /dev/null || exit 1

# Run server
CMD ["./iot_honeypot_server"]

