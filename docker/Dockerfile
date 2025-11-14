# Multi-stage Dockerfile for Raspberry Pi (ARM) and development
FROM arm32v7/ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libboost-all-dev \
    protobuf-compiler \
    libprotobuf-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Copy project files
COPY . .

# Compile the project
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Final stage for product
FROM arm32v7/ubuntu:22.04

# Installs only runtime dependencies
RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libprotoc23 \
    libprotobuf23 \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user for security
RUN useradd -m appuser && mkdir /app && chown appuser:appuser /app
USER appuser
WORKDIR /app

# Copy compiled binary
COPY --from=builder --chown=appuser:appuser /workspace/build/iot_honeypot_server .

# Copy settings if they exist.
COPY --chown=appuser:appuser configs/ ./configs/

# Expose doors
EXPOSE 502/tcp    # Modbus
EXPOSE 5683/udp   # CoAP

# Health check (optional)
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD netstat -an | grep 502 > /dev/null; if [ 0 != $? ]; then exit 1; fi;

# Run server
CMD ["./iot_honeypot_server"]