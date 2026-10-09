# ── Stage 1: Build ──────────────────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_FORCE_SYSTEM_BINARIES=1

# Install build tools and Drogon/libsodium system dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake git curl zip unzip tar pkg-config \
    libssl-dev zlib1g-dev libjsoncpp-dev uuid-dev \
    libpq-dev libsodium-dev \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /vcpkg \
    && /vcpkg/bootstrap-vcpkg.sh -disableMetrics

# Install vcpkg dependencies
RUN /vcpkg/vcpkg install drogon nlohmann-json spdlog libsodium --triplet x64-linux

WORKDIR /app
COPY . .

# Fix CMakeLists.txt sodium path for Linux (use system libsodium)
RUN sed -i 's|find_library(SODIUM_LIBRARY.*||' CMakeLists.txt \
    && sed -i '/NAMES sodium libsodium/d' CMakeLists.txt \
    && sed -i '/PATHS.*vcpkg/d' CMakeLists.txt \
    && sed -i '/REQUIRED/{ /find_library/!{ /SODIUM/d } }' CMakeLists.txt

# Build
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DVCPKG_TARGET_TRIPLET=x64-linux \
    && cmake --build build --config Release -j$(nproc)

# ── Stage 2: Runtime ─────────────────────────────────────────────────────────
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libssl3 zlib1g libjsoncpp25 libpq5 libsodium23 libuuid1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/build/HariMart ./HariMart
COPY --from=builder /app/frontend ./frontend
COPY --from=builder /app/db ./db

# Entrypoint script generates config.json from env vars then starts the app
COPY docker-entrypoint.sh ./docker-entrypoint.sh
RUN chmod +x ./docker-entrypoint.sh

EXPOSE 8080
ENTRYPOINT ["./docker-entrypoint.sh"]
