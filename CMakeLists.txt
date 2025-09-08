cmake_minimum_required(VERSION 3.20)
project(RouterSim VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# FRR Integration
pkg_check_modules(FRR REQUIRED frr)
if(FRR_FOUND)
    message(STATUS "FRR found: ${FRR_VERSION}")
    include_directories(${FRR_INCLUDE_DIRS})
    link_directories(${FRR_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "FRR not found. Please install FRR development packages.")
endif()

# ClickHouse C++ Client
find_path(CLICKHOUSE_INCLUDE_DIR clickhouse/client.h
    PATHS /usr/include /usr/local/include /opt/clickhouse/include
)
find_library(CLICKHOUSE_LIBRARY clickhouse
    PATHS /usr/lib /usr/local/lib /opt/clickhouse/lib
)

if(CLICKHOUSE_INCLUDE_DIR AND CLICKHOUSE_LIBRARY)
    message(STATUS "ClickHouse found: ${CLICKHOUSE_LIBRARY}")
    set(CLICKHOUSE_FOUND TRUE)
else()
    message(WARNING "ClickHouse not found. Building without ClickHouse support.")
    set(CLICKHOUSE_FOUND FALSE)
endif()

# YAML-CPP
find_package(yaml-cpp REQUIRED)

# Google Test
find_package(GTest REQUIRED)

# PCAP library
find_package(PkgConfig REQUIRED)
pkg_check_modules(PCAP REQUIRED libpcap)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FRR_INCLUDE_DIRS}
    ${PCAP_INCLUDE_DIRS}
)

if(CLICKHOUSE_FOUND)
    include_directories(${CLICKHOUSE_INCLUDE_DIR})
endif()

# Source files
set(ROUTER_SOURCES
    src/main.cpp
    src/router_core.cpp
    src/protocols/bgp.cpp
    src/protocols/ospf.cpp
    src/protocols/isis.cpp
    src/traffic_shaping/token_bucket.cpp
    src/traffic_shaping/wfq.cpp
    src/traffic_shaping/traffic_shaper.cpp
    src/network_impairments/netem.cpp
    src/impairments/netem.cpp
    src/frr_integration/frr_integration.cpp
    src/frr_integration/frr_client.cpp
    src/cli/cli_interface.cpp
    src/yaml_config.cpp
    src/testing/pcap_diff.cpp
    src/testing/coverage.cpp
)

# ClickHouse integration sources
if(CLICKHOUSE_FOUND)
    list(APPEND ROUTER_SOURCES
        src/analytics/clickhouse_client.cpp
        src/analytics/metrics_collector.cpp
    )
endif()

# Create executable
add_executable(router_sim ${ROUTER_SOURCES})

# Link libraries
target_link_libraries(router_sim
    ${FRR_LIBRARIES}
    ${PCAP_LIBRARIES}
    yaml-cpp
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(CLICKHOUSE_FOUND)
    target_link_libraries(router_sim ${CLICKHOUSE_LIBRARY})
    target_compile_definitions(router_sim PRIVATE CLICKHOUSE_ENABLED)
endif()

# Test executable
add_executable(test_router
    tests/test_router_core.cpp
    tests/test_traffic_shaping.cpp
    src/protocols/bgp.cpp
    src/protocols/ospf.cpp
    src/protocols/isis.cpp
    src/traffic_shaping/token_bucket.cpp
    src/traffic_shaping/wfq.cpp
    src/traffic_shaping/traffic_shaper.cpp
    src/network_impairments/netem.cpp
    src/frr_integration/frr_integration.cpp
    src/yaml_config.cpp
)

target_link_libraries(test_router
    ${FRR_LIBRARIES}
    ${PCAP_LIBRARIES}
    yaml-cpp
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(CLICKHOUSE_FOUND)
    target_link_libraries(test_router ${CLICKHOUSE_LIBRARY})
    target_compile_definitions(test_router PRIVATE CLICKHOUSE_ENABLED)
endif()

# Rust integration
find_program(CARGO cargo)
if(CARGO)
    message(STATUS "Found Cargo: ${CARGO}")
    
    # Build Rust analytics engine
    add_custom_target(rust_analytics ALL
        COMMAND ${CARGO} build --release --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/rust/Cargo.toml
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building Rust analytics engine"
    )
    
    # Link Rust library to main executable
    find_library(RUST_ANALYTICS_LIB
        NAMES analytics
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/rust/target/release
        NO_DEFAULT_PATH
    )
    
    if(RUST_ANALYTICS_LIB)
        target_link_libraries(router_sim ${RUST_ANALYTICS_LIB})
        target_compile_definitions(router_sim PRIVATE RUST_ANALYTICS_ENABLED)
    endif()
else()
    message(WARNING "Cargo not found. Building without Rust analytics engine.")
endif()

# Go API server
find_program(GO go)
if(GO)
    message(STATUS "Found Go: ${GO}")
    
    # Build Go API server
    add_custom_target(go_api_server ALL
        COMMAND ${GO} build -o ${CMAKE_CURRENT_BINARY_DIR}/api-server ./go/cmd/server
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building Go API server"
    )
else()
    message(WARNING "Go not found. Building without Go API server.")
endif()

# Install targets
install(TARGETS router_sim test_router
    RUNTIME DESTINATION bin
)

# Create config directory
install(DIRECTORY config/
    DESTINATION etc/router-sim
    FILES_MATCHING PATTERN "*.yaml" PATTERN "*.yml"
)

# Create scenarios directory
install(DIRECTORY scenarios/
    DESTINATION share/router-sim/scenarios
    FILES_MATCHING PATTERN "*.yaml" PATTERN "*.yml"
)

# CPack configuration
set(CPACK_PACKAGE_NAME "RouterSim")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multi-Protocol Router Simulator with FRR Integration")
set(CPACK_PACKAGE_VENDOR "RouterSim Team")

include(CPack)

# Custom targets
add_custom_target(run_tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_router
    DEPENDS test_router
    COMMENT "Running test suite"
)

add_custom_target(run_benchmarks
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/router_sim --benchmark
    DEPENDS router_sim
    COMMENT "Running performance benchmarks"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/router_sim
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/test_router
    COMMENT "Cleaning all build artifacts"
)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in
                   ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
    )
endif()

# Print configuration summary
message(STATUS "=== RouterSim Configuration Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "FRR integration: ${FRR_FOUND}")
message(STATUS "ClickHouse integration: ${CLICKHOUSE_FOUND}")
message(STATUS "Rust analytics: ${CARGO_FOUND}")
message(STATUS "Go API server: ${GO_FOUND}")
message(STATUS "Documentation: ${DOXYGEN_FOUND}")
message(STATUS "=======================================")
