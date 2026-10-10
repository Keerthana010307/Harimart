# ── Stage 1: Build ──────────────────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    zlib1g-dev \
    libjsoncpp-dev \
    uuid-dev \
    libpq-dev \
    libsodium-dev \
    libhiredis-dev \
    libc-ares-dev \
    libboost-dev \
    nlohmann-json3-dev \
    libspdlog-dev \
    libfmt-dev \
    && rm -rf /var/lib/apt/lists/*

# Build Drogon from source
RUN git clone --depth 1 --branch v1.9.5 https://github.com/drogonframework/drogon.git /drogon \
    && cd /drogon \
    && git submodule update --init \
    && cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_CTL=OFF \
        -DUSE_POSTGRESQL=ON \
        -DUSE_MYSQL=OFF \
        -DUSE_SQLITE3=OFF \
    && cmake --build build -j1 \
    && cmake --install build

WORKDIR /app
COPY . .

# Use the Linux-specific CMakeLists (avoids Windows CRLF heredoc issues)
RUN cp CMakeLists.linux.txt CMakeLists.txt

# Build HariMart
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j2

# ── Stage 2: Runtime ─────────────────────────────────────────────────────────
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libssl3 \
    zlib1g \
    libjsoncpp25 \
    libpq5 \
    libsodium23 \
    libuuid1 \
    libc-ares2 \
    libhiredis0.14 \
    postgresql-client \
    dnsutils \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/HariMart ./HariMart
COPY --from=builder /app/frontend ./frontend
COPY --from=builder /app/db ./db
COPY --from=builder /usr/local/lib/libdrogon* /usr/local/lib/
COPY --from=builder /usr/local/lib/libtrantor* /usr/local/lib/
RUN ldconfig

ENV ENTRYPOINT_VERSION=5
COPY docker-entrypoint.sh ./docker-entrypoint.sh
RUN chmod +x ./docker-entrypoint.sh

EXPOSE 8080
ENTRYPOINT ["./docker-entrypoint.sh"]
