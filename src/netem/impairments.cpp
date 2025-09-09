#include "network_impairments.h"
#include <iostream>
#include <random>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace RouterSim {

NetworkImpairments::NetworkImpairments() 
    : enabled_(false), delay_ms_(0), jitter_ms_(0), loss_rate_(0.0), 
      duplicate_rate_(0.0), reorder_rate_(0.0), corrupt_rate_(0.0),
      random_engine_(std::chrono::steady_clock::now().time_since_epoch().count()) {
    
    std::cout << "NetworkImpairments initialized\n";
}

NetworkImpairments::~NetworkImpairments() {
    disable_impairments();
}

bool NetworkImpairments::enable_impairments(const std::string& interface) {
    if (enabled_) {
        disable_impairments();
    }
    
    interface_ = interface;
    
    // Build tc command for netem
    std::stringstream tc_cmd;
    tc_cmd << "sudo tc qdisc add dev " << interface_ << " root netem";
    
    bool has_impairments = false;
    
    if (delay_ms_ > 0) {
        tc_cmd << " delay " << delay_ms_ << "ms";
        if (jitter_ms_ > 0) {
            tc_cmd << " " << jitter_ms_ << "ms";
        }
        has_impairments = true;
    }
    
    if (loss_rate_ > 0.0) {
        if (has_impairments) tc_cmd << " ";
        tc_cmd << " loss " << (loss_rate_ * 100.0) << "%";
        has_impairments = true;
    }
    
    if (duplicate_rate_ > 0.0) {
        if (has_impairments) tc_cmd << " ";
        tc_cmd << " duplicate " << (duplicate_rate_ * 100.0) << "%";
        has_impairments = true;
    }
    
    if (reorder_rate_ > 0.0) {
        if (has_impairments) tc_cmd << " ";
        tc_cmd << " reorder " << (reorder_rate_ * 100.0) << "%";
        has_impairments = true;
    }
    
    if (corrupt_rate_ > 0.0) {
        if (has_impairments) tc_cmd << " ";
        tc_cmd << " corrupt " << (corrupt_rate_ * 100.0) << "%";
        has_impairments = true;
    }
    
    if (!has_impairments) {
        std::cout << "No impairments configured\n";
        return true;
    }
    
    // Execute tc command
    int result = system(tc_cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to apply network impairments: " << tc_cmd.str() << "\n";
        return false;
    }
    
    enabled_ = true;
    std::cout << "Network impairments enabled on " << interface_ << "\n";
    std::cout << "Command: " << tc_cmd.str() << "\n";
    return true;
}

bool NetworkImpairments::disable_impairments() {
    if (!enabled_ || interface_.empty()) {
        return true;
    }
    
    std::string cmd = "sudo tc qdisc del dev " + interface_ + " root";
    int result = system(cmd.c_str());
    
    if (result == 0) {
        enabled_ = false;
        std::cout << "Network impairments disabled on " << interface_ << "\n";
    } else {
        std::cerr << "Failed to disable network impairments\n";
    }
    
    return result == 0;
}

void NetworkImpairments::set_delay(uint32_t delay_ms, uint32_t jitter_ms) {
    delay_ms_ = delay_ms;
    jitter_ms_ = jitter_ms;
    std::cout << "Delay set to " << delay_ms_ << "ms";
    if (jitter_ms_ > 0) {
        std::cout << " ±" << jitter_ms_ << "ms";
    }
    std::cout << "\n";
}

void NetworkImpairments::set_loss_rate(double rate) {
    loss_rate_ = std::max(0.0, std::min(1.0, rate));
    std::cout << "Loss rate set to " << (loss_rate_ * 100.0) << "%\n";
}

void NetworkImpairments::set_duplicate_rate(double rate) {
    duplicate_rate_ = std::max(0.0, std::min(1.0, rate));
    std::cout << "Duplicate rate set to " << (duplicate_rate_ * 100.0) << "%\n";
}

void NetworkImpairments::set_reorder_rate(double rate) {
    reorder_rate_ = std::max(0.0, std::min(1.0, rate));
    std::cout << "Reorder rate set to " << (reorder_rate_ * 100.0) << "%\n";
}

void NetworkImpairments::set_corrupt_rate(double rate) {
    corrupt_rate_ = std::max(0.0, std::min(1.0, rate));
    std::cout << "Corrupt rate set to " << (corrupt_rate_ * 100.0) << "%\n";
}

bool NetworkImpairments::apply_packet_impairments(Packet& packet) {
    if (!enabled_) {
        return true;
    }
    
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    // Apply packet loss
    if (loss_rate_ > 0.0 && dist(random_engine_) < loss_rate_) {
        std::cout << "Packet dropped due to loss simulation\n";
        return false;
    }
    
    // Apply packet duplication
    if (duplicate_rate_ > 0.0 && dist(random_engine_) < duplicate_rate_) {
        // Note: In a real implementation, this would create a duplicate packet
        std::cout << "Packet duplicated\n";
    }
    
    // Apply packet corruption
    if (corrupt_rate_ > 0.0 && dist(random_engine_) < corrupt_rate_) {
        corrupt_packet(packet);
        std::cout << "Packet corrupted\n";
    }
    
    // Apply delay (simulated)
    if (delay_ms_ > 0) {
        uint32_t actual_delay = delay_ms_;
        if (jitter_ms_ > 0) {
            std::uniform_int_distribution<uint32_t> jitter_dist(0, jitter_ms_ * 2);
            actual_delay += jitter_dist(random_engine_) - jitter_ms_;
        }
        
        // In a real implementation, this would delay the packet
        std::this_thread::sleep_for(std::chrono::milliseconds(actual_delay));
        std::cout << "Packet delayed by " << actual_delay << "ms\n";
    }
    
    return true;
}

void NetworkImpairments::corrupt_packet(Packet& packet) {
    if (packet.data.empty()) {
        return;
    }
    
    std::uniform_int_distribution<size_t> pos_dist(0, packet.data.size() - 1);
    std::uniform_int_distribution<uint8_t> byte_dist(0, 255);
    
    // Corrupt a random byte
    size_t pos = pos_dist(random_engine_);
    packet.data[pos] = byte_dist(random_engine_);
}

bool NetworkImpairments::is_enabled() const {
    return enabled_;
}

std::string NetworkImpairments::get_interface() const {
    return interface_;
}

NetworkImpairments::ImpairmentConfig NetworkImpairments::get_config() const {
    ImpairmentConfig config;
    config.delay_ms = delay_ms_;
    config.jitter_ms = jitter_ms_;
    config.loss_rate = loss_rate_;
    config.duplicate_rate = duplicate_rate_;
    config.reorder_rate = reorder_rate_;
    config.corrupt_rate = corrupt_rate_;
    return config;
}

void NetworkImpairments::set_config(const ImpairmentConfig& config) {
    set_delay(config.delay_ms, config.jitter_ms);
    set_loss_rate(config.loss_rate);
    set_duplicate_rate(config.duplicate_rate);
    set_reorder_rate(config.reorder_rate);
    set_corrupt_rate(config.corrupt_rate);
}

// Bandwidth Limiter implementation
BandwidthLimiter::BandwidthLimiter() 
    : enabled_(false), rate_bps_(0), interface_("") {
    std::cout << "BandwidthLimiter initialized\n";
}

BandwidthLimiter::~BandwidthLimiter() {
    disable_limiting();
}

bool BandwidthLimiter::enable_limiting(const std::string& interface, uint64_t rate_bps) {
    if (enabled_) {
        disable_limiting();
    }
    
    interface_ = interface;
    rate_bps_ = rate_bps;
    
    // Convert bps to kbps for tc
    uint64_t rate_kbps = rate_bps_ / 1000;
    if (rate_kbps == 0) rate_kbps = 1; // Minimum 1 kbps
    
    std::stringstream tc_cmd;
    tc_cmd << "sudo tc qdisc add dev " << interface_ << " root tbf";
    tc_cmd << " rate " << rate_kbps << "kbit";
    tc_cmd << " burst " << (rate_kbps / 10) << "kbit";
    tc_cmd << " latency 50ms";
    
    int result = system(tc_cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to apply bandwidth limiting: " << tc_cmd.str() << "\n";
        return false;
    }
    
    enabled_ = true;
    std::cout << "Bandwidth limiting enabled on " << interface_ 
              << " at " << rate_bps_ << " bps\n";
    return true;
}

bool BandwidthLimiter::disable_limiting() {
    if (!enabled_ || interface_.empty()) {
        return true;
    }
    
    std::string cmd = "sudo tc qdisc del dev " + interface_ + " root";
    int result = system(cmd.c_str());
    
    if (result == 0) {
        enabled_ = false;
        std::cout << "Bandwidth limiting disabled on " << interface_ << "\n";
    } else {
        std::cerr << "Failed to disable bandwidth limiting\n";
    }
    
    return result == 0;
}

void BandwidthLimiter::set_rate(uint64_t rate_bps) {
    rate_bps_ = rate_bps;
    
    if (enabled_) {
        // Reapply with new rate
        disable_limiting();
        enable_limiting(interface_, rate_bps_);
    }
    
    std::cout << "Bandwidth rate updated to " << rate_bps_ << " bps\n";
}

bool BandwidthLimiter::is_enabled() const {
    return enabled_;
}

uint64_t BandwidthLimiter::get_rate() const {
    return rate_bps_;
}

std::string BandwidthLimiter::get_interface() const {
    return interface_;
}

// Network Emulator Manager
NetworkEmulator::NetworkEmulator() 
    : impairments_(), bandwidth_limiter_() {
    std::cout << "NetworkEmulator initialized\n";
}

NetworkEmulator::~NetworkEmulator() {
    disable_all();
}

bool NetworkEmulator::configure_impairments(const std::string& interface,
                                           const ImpairmentConfig& config) {
    impairments_.set_config(config);
    return impairments_.enable_impairments(interface);
}

bool NetworkEmulator::configure_bandwidth_limit(const std::string& interface,
                                               uint64_t rate_bps) {
    return bandwidth_limiter_.enable_limiting(interface, rate_bps);
}

bool NetworkEmulator::disable_all() {
    bool success = true;
    
    if (impairments_.is_enabled()) {
        success &= impairments_.disable_impairments();
    }
    
    if (bandwidth_limiter_.is_enabled()) {
        success &= bandwidth_limiter_.disable_limiting();
    }
    
    return success;
}

bool NetworkEmulator::apply_impairments(Packet& packet) {
    return impairments_.apply_packet_impairments(packet);
}

bool NetworkEmulator::is_impairments_enabled() const {
    return impairments_.is_enabled();
}

bool NetworkEmulator::is_bandwidth_limited() const {
    return bandwidth_limiter_.is_enabled();
}

NetworkEmulator::Status NetworkEmulator::get_status() const {
    Status status;
    status.impairments_enabled = impairments_.is_enabled();
    status.bandwidth_limited = bandwidth_limiter_.is_enabled();
    status.impairment_config = impairments_.get_config();
    status.bandwidth_rate = bandwidth_limiter_.get_rate();
    status.interface = impairments_.get_interface();
    return status;
}

} // namespace RouterSim