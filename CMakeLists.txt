cmake_minimum_required(VERSION 3.25)

project(HariMart
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)

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
        unofficial-sodium::sodium
)

if(MSVC)
    target_compile_options(HariMart PRIVATE /W4)
else()
    target_compile_options(HariMart PRIVATE -Wall -Wextra -Wpedantic)
endif()