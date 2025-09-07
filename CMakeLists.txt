cmake_minimum_required(VERSION 3.16)
project(RouterSim VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Find FRR libraries
pkg_check_modules(FRR REQUIRED libfrr)

# Find libpcap
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPCAP REQUIRED libpcap)

# Find Google Test
find_package(GTest REQUIRED)

# Find yaml-cpp
find_package(yaml-cpp REQUIRED)

# Find ZeroMQ
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${FRR_INCLUDE_DIRS})
include_directories(${LIBPCAP_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/router_core.cpp
    src/frr_integration/frr_client.cpp
    src/protocols/bgp.cpp
    src/protocols/ospf.cpp
    src/protocols/isis.cpp
    src/traffic_shaping/traffic_shaper.cpp
    src/netem/impairments.cpp
    src/cli/cli_interface.cpp
    src/yaml/scenario_parser.cpp
    src/testing/pcap_diff.cpp
)

# Header files
set(HEADERS
    include/router_core.h
    include/frr_integration/frr_client.h
    include/protocols/bgp.h
    include/protocols/ospf.h
    include/protocols/isis.h
    include/protocols/protocol_interface.h
    include/traffic_shaping/traffic_shaper.h
    include/netem/impairments.h
    include/cli/cli_interface.h
    include/yaml/scenario_parser.h
    include/testing/pcap_diff.h
)

# Create executable
add_executable(router_sim ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(router_sim 
    ${FRR_LIBRARIES}
    ${LIBPCAP_LIBRARIES}
    ${ZMQ_LIBRARIES}
    Threads::Threads
    yaml-cpp
)

# Compiler flags
target_compile_options(router_sim PRIVATE ${FRR_CFLAGS_OTHER})
target_compile_options(router_sim PRIVATE ${LIBPCAP_CFLAGS_OTHER})
target_compile_options(router_sim PRIVATE ${ZMQ_CFLAGS_OTHER})

# Test executable
add_executable(router_tests 
    tests/test_router_core.cpp
)

target_link_libraries(router_tests 
    GTest::GTest
    GTest::Main
    ${FRR_LIBRARIES}
    ${LIBPCAP_LIBRARIES}
    ${ZMQ_LIBRARIES}
    Threads::Threads
    yaml-cpp
)

# Enable testing
enable_testing()
add_test(NAME RouterSimTests COMMAND router_tests)

# Installation
install(TARGETS router_sim router_tests DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include/router_sim)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_SOURCE_DIR}/docs/Doxyfile @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()
