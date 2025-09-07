#include "router_sim.h"
#include "cli_interface.h"
#include "yaml_config.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

using namespace RouterSim;

// Global router simulator instance
std::unique_ptr<RouterSimulator> g_router_sim = nullptr;

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    if (g_router_sim) {
        g_router_sim->stop();
    }
    exit(0);
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n"
              << "Multi-Protocol Router Simulator\n\n"
              << "Options:\n"
              << "  -c, --config FILE     Configuration file path\n"
              << "  -s, --scenario FILE   Scenario file to run\n"
              << "  -d, --daemon          Run as daemon\n"
              << "  -l, --log-level LEVEL Set log level (debug, info, warn, error)\n"
              << "  -p, --port PORT       CLI port (default: 8080)\n"
              << "  -i, --interactive     Start interactive CLI\n"
              << "  -h, --help            Show this help message\n"
              << "  -v, --version         Show version information\n\n"
              << "Examples:\n"
              << "  " << program_name << " -c config/router.yaml\n"
              << "  " << program_name << " -s scenarios/bgp_convergence.yaml\n"
              << "  " << program_name << " --daemon --port 8080\n"
              << "  " << program_name << " --interactive\n";
}

void print_version() {
    std::cout << "Multi-Protocol Router Simulator v1.0.0\n"
              << "Built with C++17, FRR integration, and traffic shaping\n"
              << "Copyright (c) 2024 Router Simulator Project\n";
}

int main(int argc, char* argv[]) {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    // Command line options
    static struct option long_options[] = {
        {"config",     required_argument, 0, 'c'},
        {"scenario",   required_argument, 0, 's'},
        {"daemon",     no_argument,       0, 'd'},
        {"log-level",  required_argument, 0, 'l'},
        {"port",       required_argument, 0, 'p'},
        {"interactive", no_argument,      0, 'i'},
        {"help",       no_argument,       0, 'h'},
        {"version",    no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    std::string config_file = "config/config.yaml";
    std::string scenario_file;
    bool daemon_mode = false;
    std::string log_level = "info";
    int cli_port = 8080;
    bool interactive = false;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "c:s:dl:p:ihv", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                config_file = optarg;
                break;
            case 's':
                scenario_file = optarg;
                break;
            case 'd':
                daemon_mode = true;
                break;
            case 'l':
                log_level = optarg;
                break;
            case 'p':
                cli_port = std::stoi(optarg);
                break;
            case 'i':
                interactive = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'v':
                print_version();
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                break;
        }
    }

    try {
        // Create router simulator instance
        g_router_sim = std::make_unique<RouterSimulator>();

        // Load configuration
        if (!g_router_sim->load_config_from_file(config_file)) {
            std::cerr << "Failed to load configuration from " << config_file << std::endl;
            return 1;
        }

        // Initialize router
        RouterConfig config;
        if (!g_router_sim->initialize(config)) {
            std::cerr << "Failed to initialize router simulator" << std::endl;
            return 1;
        }

        // Run scenario if specified
        if (!scenario_file.empty()) {
            if (!g_router_sim->load_scenario(scenario_file)) {
                std::cerr << "Failed to load scenario from " << scenario_file << std::endl;
                return 1;
            }
        }

        // Start router
        g_router_sim->start();

        if (daemon_mode) {
            // Run as daemon
            std::cout << "Router simulator running as daemon on port " << cli_port << std::endl;
            g_router_sim->start_cli();
            
            // Keep running until signal received
            while (g_router_sim->is_running()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        } else if (interactive) {
            // Start interactive CLI
            std::cout << "Starting interactive CLI..." << std::endl;
            g_router_sim->start_cli();
        } else {
            // Run scenario and exit
            if (!scenario_file.empty()) {
                std::cout << "Running scenario: " << scenario_file << std::endl;
                g_router_sim->run_scenario(scenario_file);
            } else {
                std::cout << "No scenario specified. Use -s to specify a scenario file." << std::endl;
                print_usage(argv[0]);
                return 1;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
