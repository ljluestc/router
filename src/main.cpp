#include "router_core.h"
#include "protocols/bgp.h"
#include "protocols/ospf.h"
#include "protocols/isis.h"
#include "traffic_shaping/token_bucket.h"
#include "traffic_shaping/wfq.h"
#include "network_impairments/netem.h"
#include "frr_integration/frr_control.h"
#include "analytics/clickhouse_client.h"
#include "cli/cli_interface.h"
#include "config/yaml_config.h"
#include "testing/pcap_diff.h"

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace RouterSim;

// Global router instance for signal handling
std::unique_ptr<RouterCore> g_router;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully...\n";
    if (g_router) {
        g_router->stop();
    }
}

void print_banner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║                    Multi-Protocol Router Sim                ║
║                                                              ║
║  Features:                                                   ║
║  • FRR Control Plane Integration                             ║
║  • BGP/OSPF/ISIS Protocol Support                           ║
║  • Token-Bucket & WFQ Traffic Shaping                       ║
║  • tc/netem Network Impairments                             ║
║  • ClickHouse Analytics Integration                          ║
║  • Comprehensive Test Suite with pcap diffing               ║
║  • CLI & YAML Scenario Configuration                        ║
║                                                              ║
║  Cloud Networking Concepts:                                  ║
║  • VPC Routing Simulation                                    ║
║  • NAT Gateway Functionality                                 ║
║  • Load Balancer Integration                                 ║
║  • Service Mesh Routing                                      ║
╚══════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n"
              << "Options:\n"
              << "  -c, --config FILE     Configuration file (default: config/router.yaml)\n"
              << "  -s, --scenario FILE   Scenario file to run\n"
              << "  -d, --daemon          Run as daemon\n"
              << "  -v, --verbose         Verbose output\n"
              << "  -h, --help            Show this help\n"
              << "  --test                Run test suite\n"
              << "  --benchmark           Run performance benchmarks\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    print_banner();
    
    // Parse command line arguments
    std::string config_file = "config/router.yaml";
    std::string scenario_file;
    bool daemon_mode = false;
    bool verbose = false;
    bool run_tests = false;
    bool run_benchmark = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: --config requires a filename\n";
                return 1;
            }
        } else if (arg == "-s" || arg == "--scenario") {
            if (i + 1 < argc) {
                scenario_file = argv[++i];
            } else {
                std::cerr << "Error: --scenario requires a filename\n";
                return 1;
            }
        } else if (arg == "-d" || arg == "--daemon") {
            daemon_mode = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--test") {
            run_tests = true;
        } else if (arg == "--benchmark") {
            run_benchmark = true;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Set up signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // Create router instance
        g_router = std::make_unique<RouterCore>();
        
        if (run_tests) {
            std::cout << "Running test suite...\n";
            // Run tests would be implemented here
            return 0;
        }
        
        if (run_benchmark) {
            std::cout << "Running performance benchmarks...\n";
            // Run benchmarks would be implemented here
            return 0;
        }
        
        // Initialize router
        std::cout << "Initializing router with config: " << config_file << "\n";
        if (!g_router->initialize(config_file)) {
            std::cerr << "Failed to initialize router\n";
            return 1;
        }
        
        // Load scenario if provided
        if (!scenario_file.empty()) {
            std::cout << "Loading scenario: " << scenario_file << "\n";
            // Scenario loading would be implemented here
        }
        
        // Start router
        std::cout << "Starting router...\n";
        g_router->start();
        
        if (daemon_mode) {
            std::cout << "Running as daemon (PID: " << getpid() << ")\n";
            // Daemon mode implementation
            while (g_router->is_running()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else {
            std::cout << "Router running. Press Ctrl+C to stop.\n";
            // Interactive mode
            while (g_router->is_running()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        std::cout << "Router stopped.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}