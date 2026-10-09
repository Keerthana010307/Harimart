# ── Stage 1: Build ──────────────────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install compiler, cmake, and ALL Drogon + app dependencies via apt
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

# Build and install Drogon from source (apt version is too old)
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

# Write a Linux-compatible CMakeLists.txt (no Windows vcpkg path for sodium)
RUN cat > CMakeLists.txt << 'CMEOF'
cmake_minimum_required(VERSION 3.25)

project(HariMart
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_library(SODIUM_LIBRARY NAMES sodium libsodium REQUIRED)

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(HariMart ${SOURCES})

target_include_directories(HariMart
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(HariMart
    PRIVATE
        Drogon::Drogon
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        ${SODIUM_LIBRARY}
)

target_compile_options(HariMart PRIVATE -Wall -Wextra -Wpedantic)
CMEOF

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
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/HariMart ./HariMart
COPY --from=builder /app/frontend ./frontend
COPY --from=builder /app/db ./db
COPY --from=builder /usr/local/lib/libdrogon* /usr/local/lib/
COPY --from=builder /usr/local/lib/libtrantor* /usr/local/lib/
RUN ldconfig

COPY docker-entrypoint.sh ./docker-entrypoint.sh
RUN chmod +x ./docker-entrypoint.sh

EXPOSE 8080
ENTRYPOINT ["./docker-entrypoint.sh"]
