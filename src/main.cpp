#include "router_sim.h"
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

using namespace RouterSim;

void print_usage(const char* program_name) {
    std::cout << "Multi-Protocol Router Simulator\n";
    std::cout << "Usage: " << program_name << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help              Show this help message\n";
    std::cout << "  -c, --config FILE       Load configuration from FILE\n";
    std::cout << "  -s, --scenario FILE     Execute scenario from FILE\n";
    std::cout << "  -i, --interactive       Start interactive CLI mode\n";
    std::cout << "  -d, --daemon            Run as daemon\n";
    std::cout << "  -v, --verbose           Enable verbose output\n";
    std::cout << "  -V, --version           Show version information\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " -i                    # Interactive mode\n";
    std::cout << "  " << program_name << " -c config.yaml -i     # Load config and start CLI\n";
    std::cout << "  " << program_name << " -s scenario.yaml      # Execute scenario\n";
    std::cout << "  " << program_name << " -d --config /etc/router_sim.yaml  # Daemon mode\n";
}

void print_version() {
    std::cout << "Multi-Protocol Router Simulator v1.0.0\n";
    std::cout << "Built with C++17, FRR integration, and Rust components\n";
    std::cout << "Features: BGP, OSPF, IS-IS, Traffic Shaping, Network Impairments\n";
}

int main(int argc, char* argv[]) {
    // Command line options
    static struct option long_options[] = {
        {"help",        no_argument,       0, 'h'},
        {"config",      required_argument, 0, 'c'},
        {"scenario",    required_argument, 0, 's'},
        {"interactive", no_argument,       0, 'i'},
        {"daemon",      no_argument,       0, 'd'},
        {"verbose",     no_argument,       0, 'v'},
        {"version",     no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };

    std::string config_file;
    std::string scenario_file;
    bool interactive_mode = false;
    bool daemon_mode = false;
    bool verbose = false;

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "hc:s:idvV", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'c':
                config_file = optarg;
                break;
            case 's':
                scenario_file = optarg;
                break;
            case 'i':
                interactive_mode = true;
                break;
            case 'd':
                daemon_mode = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 'V':
                print_version();
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                abort();
        }
    }

    try {
        // Create router simulator
        RouterSimulator router;

        // Load configuration if provided
        if (!config_file.empty()) {
            std::cout << "Loading configuration from: " << config_file << std::endl;
            if (!router.load_config(config_file)) {
                std::cerr << "Failed to load configuration from: " << config_file << std::endl;
                return 1;
            }
        } else {
            // Use default configuration
            RouterConfig default_config;
            default_config.router_id = "1.1.1.1";
            default_config.hostname = "router-sim";
            default_config.enable_bgp = true;
            default_config.enable_ospf = true;
            default_config.as_number = 65001;
            default_config.area_id = "0.0.0.0";
            
            if (!router.initialize(default_config)) {
                std::cerr << "Failed to initialize router with default configuration" << std::endl;
                return 1;
            }
        }

        // Start the router
        if (!router.start()) {
            std::cerr << "Failed to start router simulator" << std::endl;
            return 1;
        }

        std::cout << "Router simulator started successfully" << std::endl;

        // Execute scenario if provided
        if (!scenario_file.empty()) {
            std::cout << "Executing scenario from: " << scenario_file << std::endl;
            // TODO: Implement scenario execution
            std::cout << "Scenario execution completed" << std::endl;
        }

        // Start interactive mode if requested
        if (interactive_mode) {
            std::cout << "Starting interactive CLI mode..." << std::endl;
            std::cout << "Type 'help' for available commands or 'exit' to quit" << std::endl;
            
            // Start CLI interface
            auto cli = router.get_cli_interface();
            if (cli) {
                cli->start_interactive_mode();
            } else {
                std::cerr << "CLI interface not available" << std::endl;
                return 1;
            }
        } else if (daemon_mode) {
            std::cout << "Running in daemon mode..." << std::endl;
            std::cout << "Press Ctrl+C to stop" << std::endl;
            
            // Keep running until interrupted
            while (router.is_running()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else {
            // Default: run for a short time then exit
            std::cout << "Running for 10 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        // Stop the router
        router.stop();
        std::cout << "Router simulator stopped" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}