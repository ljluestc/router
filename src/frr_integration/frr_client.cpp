#include "frr_integration.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

namespace RouterSim {

FRRIntegration::FRRIntegration() 
    : initialized_(false), frr_running_(false), monitor_running_(false) {
    // Initialize protocol status
    protocol_status_[Protocol::BGP] = false;
    protocol_status_[Protocol::OSPF] = false;
    protocol_status_[Protocol::ISIS] = false;
}

FRRIntegration::~FRRIntegration() {
    shutdown();
}

bool FRRIntegration::initialize() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (initialized_.load()) {
        return true;
    }

    std::cout << "Initializing FRR integration...\n";

    // Check if FRR is installed
    if (system("which vtysh > /dev/null 2>&1") != 0) {
        std::cerr << "Error: FRR not found. Please install FRR.\n";
        return false;
    }

    // Start FRR daemon
    if (!start_frr_daemon()) {
        std::cerr << "Error: Failed to start FRR daemon\n";
        return false;
    }

    // Start monitoring thread
    monitor_running_.store(true);
    monitor_thread_ = std::thread(&FRRIntegration::monitor_loop, this);

    initialized_.store(true);
    std::cout << "FRR integration initialized successfully\n";
    return true;
}

void FRRIntegration::shutdown() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!initialized_.load()) {
        return;
    }

    std::cout << "Shutting down FRR integration...\n";

    // Stop monitoring
    monitor_running_.store(false);
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }

    // Stop all protocols
    stop_bgp();
    stop_ospf();
    stop_isis();

    // Stop FRR daemon
    stop_frr_daemon();

    initialized_.store(false);
    std::cout << "FRR integration shutdown complete\n";
}

bool FRRIntegration::start_frr_daemon() {
    // Check if FRR is already running
    if (is_frr_running()) {
        frr_running_.store(true);
        return true;
    }

    // Start FRR daemon
    int result = system("sudo systemctl start frr");
    if (result != 0) {
        std::cerr << "Failed to start FRR daemon\n";
        return false;
    }

    // Wait for FRR to start
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (is_frr_running()) {
            frr_running_.store(true);
            return true;
        }
    }

    std::cerr << "FRR daemon failed to start within timeout\n";
    return false;
}

bool FRRIntegration::stop_frr_daemon() {
    if (!frr_running_.load()) {
        return true;
    }

    int result = system("sudo systemctl stop frr");
    frr_running_.store(false);
    return result == 0;
}

bool FRRIntegration::is_frr_running() const {
    int result = system("systemctl is-active --quiet frr");
    return result == 0;
}

} // namespace RouterSim