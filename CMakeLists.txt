cmake_minimum_required(VERSION 3.16)
project(RouterSim VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# FRR dependencies
pkg_check_modules(FRR REQUIRED frr)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7180a0f7f0b2c0b4f4a2.zip
)
FetchContent_MakeAvailable(googletest)

# YAML parser
find_package(yaml-cpp REQUIRED)

# Include directories
include_directories(include)
include_directories(src)

# Source files
set(SOURCES
    src/main.cpp
    src/router_core.cpp
    src/frr_integration.cpp
    src/traffic_shaping.cpp
    src/netem_impairments.cpp
    src/cli_interface.cpp
    src/yaml_config.cpp
    src/packet_processor.cpp
    src/routing_table.cpp
    src/statistics.cpp
    src/protocols/bgp.cpp
    src/protocols/ospf.cpp
    src/protocols/isis.cpp
)

# Create executable
add_executable(router_sim ${SOURCES})

# Link libraries
target_link_libraries(router_sim
    ${FRR_LIBRARIES}
    yaml-cpp
    Threads::Threads
    rt
    pthread
)

# Compiler flags
target_compile_options(router_sim PRIVATE ${FRR_CFLAGS_OTHER})

# Test executable
add_executable(router_test
    tests/test_main.cpp
    tests/test_router_core.cpp
    tests/test_traffic_shaping.cpp
    tests/test_frr_integration.cpp
    tests/test_netem.cpp
    tests/test_packet_processor.cpp
    src/router_core.cpp
    src/frr_integration.cpp
    src/traffic_shaping.cpp
    src/netem_impairments.cpp
    src/packet_processor.cpp
    src/routing_table.cpp
    src/statistics.cpp
    src/protocols/bgp.cpp
    src/protocols/ospf.cpp
    src/protocols/isis.cpp
)

target_link_libraries(router_test
    gtest_main
    gtest
    ${FRR_LIBRARIES}
    yaml-cpp
    Threads::Threads
    rt
    pthread
)

target_compile_options(router_test PRIVATE ${FRR_CFLAGS_OTHER})

# Enable testing
enable_testing()
add_test(NAME RouterTests COMMAND router_test)