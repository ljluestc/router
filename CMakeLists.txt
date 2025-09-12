cmake_minimum_required(VERSION 3.16)
project(RouterSim VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

# FRR dependencies (optional)
pkg_check_modules(FRR frr)
if(FRR_FOUND)
    message(STATUS "FRR found - enabling FRR integration")
    add_definitions(-DHAVE_FRR)
else()
    message(WARNING "FRR not found - building without FRR integration")
    message(STATUS "To enable FRR integration, install libfrr-dev and frr-dev packages")
endif()

# Network libraries
find_library(PCAP_LIB pcap)
find_library(NETLINK_LIB nl-3)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${FRR_INCLUDE_DIRS}
)

# Source files
set(ROUTER_SOURCES
    src/main.cpp
    src/router_core.cpp
    src/protocols/bgp.cpp
    src/protocols/ospf.cpp
    src/protocols/isis.cpp
    src/traffic_shaping/token_bucket.cpp
    src/traffic_shaping/wfq.cpp
    src/impairments/netem.cpp
    src/cli/cli_interface.cpp
    src/config/yaml_config.cpp
    src/testing/pcap_diff.cpp
    src/testing/coverage.cpp
)

# Create main executable
add_executable(router_sim ${ROUTER_SOURCES})

# Link libraries
target_link_libraries(router_sim
    ${PCAP_LIB}
    ${NETLINK_LIB}
    Threads::Threads
    yaml-cpp
)

# Add FRR libraries if available
if(FRR_FOUND)
    target_link_libraries(router_sim ${FRR_LIBRARIES})
    target_include_directories(router_sim PRIVATE ${FRR_INCLUDE_DIRS})
endif()

# Test executable
add_executable(router_tests
    tests/test_router_core.cpp
    tests/test_protocols.cpp
    tests/test_traffic_shaping.cpp
    tests/test_impairments.cpp
    tests/test_cli.cpp
    tests/test_config.cpp
)

target_link_libraries(router_tests
    GTest::GTest
    GTest::Main
    ${FRR_LIBRARIES}
    ${PCAP_LIB}
    ${NETLINK_LIB}
    Threads::Threads
    yaml-cpp
)

# Enable testing
enable_testing()
add_test(NAME router_tests COMMAND router_tests)

# Coverage support
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Installation
install(TARGETS router_sim router_tests
    RUNTIME DESTINATION bin
)

install(DIRECTORY scenarios/ DESTINATION share/router_sim/scenarios)
install(DIRECTORY docs/ DESTINATION share/router_sim/docs)
