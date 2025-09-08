#include "impairments.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>

namespace RouterSim {

NetworkImpairments::NetworkImpairments() 
    : enabled_(false), interface_("lo"), 
      delay_ms_(0), jitter_ms_(0), loss_percent_(0.0),
      duplicate_percent_(0.0), reorder_percent_(0.0),
      corruption_percent_(0.0), rate_limit_bps_(0) {
}

NetworkImpairments::~NetworkImpairments() {
    disable_impairments();
}

bool NetworkImpairments::enable_impairments() {
    if (enabled_) {
        return true;
    }
    
    // Build tc command
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " root netem";
    
    bool has_impairments = false;
    
    if (delay_ms_ > 0) {
        cmd << " delay " << delay_ms_ << "ms";
        if (jitter_ms_ > 0) {
            cmd << " " << jitter_ms_ << "ms";
        }
        has_impairments = true;
    }
    
    if (loss_percent_ > 0.0) {
        if (has_impairments) cmd << " ";
        cmd << " loss " << loss_percent_ << "%";
        has_impairments = true;
    }
    
    if (duplicate_percent_ > 0.0) {
        if (has_impairments) cmd << " ";
        cmd << " duplicate " << duplicate_percent_ << "%";
        has_impairments = true;
    }
    
    if (reorder_percent_ > 0.0) {
        if (has_impairments) cmd << " ";
        cmd << " reorder " << reorder_percent_ << "%";
        has_impairments = true;
    }
    
    if (corruption_percent_ > 0.0) {
        if (has_impairments) cmd << " ";
        cmd << " corrupt " << corruption_percent_ << "%";
        has_impairments = true;
    }
    
    if (rate_limit_bps_ > 0) {
        if (has_impairments) cmd << " ";
        cmd << " rate " << (rate_limit_bps_ / 1000) << "kbit";
        has_impairments = true;
    }
    
    if (!has_impairments) {
        std::cout << "No impairments configured" << std::endl;
        return false;
    }
    
    // Execute tc command
    int result = system(cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to enable network impairments: " << cmd.str() << std::endl;
        return false;
    }
    
    enabled_ = true;
    std::cout << "Network impairments enabled: " << cmd.str() << std::endl;
    return true;
}

bool NetworkImpairments::disable_impairments() {
    if (!enabled_) {
        return true;
    }
    
    // Remove netem qdisc
    std::stringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = system(cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to disable network impairments" << std::endl;
        return false;
    }
    
    enabled_ = false;
    std::cout << "Network impairments disabled" << std::endl;
    return true;
}

bool NetworkImpairments::update_impairments() {
    if (disable_impairments()) {
        return enable_impairments();
    }
    return false;
}

void NetworkImpairments::set_delay(uint32_t delay_ms, uint32_t jitter_ms) {
    delay_ms_ = delay_ms;
    jitter_ms_ = jitter_ms;
    
    if (enabled_) {
        update_impairments();
    }
}

void NetworkImpairments::set_loss(double loss_percent) {
    loss_percent_ = std::max(0.0, std::min(100.0, loss_percent));
    
    if (enabled_) {
        update_impairments();
    }
}

void NetworkImpairments::set_duplicate(double duplicate_percent) {
    duplicate_percent_ = std::max(0.0, std::min(100.0, duplicate_percent));
    
    if (enabled_) {
        update_impairments();
    }
}

void NetworkImpairments::set_reorder(double reorder_percent) {
    reorder_percent_ = std::max(0.0, std::min(100.0, reorder_percent));
    
    if (enabled_) {
        update_impairments();
    }
}

void NetworkImpairments::set_corruption(double corruption_percent) {
    corruption_percent_ = std::max(0.0, std::min(100.0, corruption_percent));
    
    if (enabled_) {
        update_impairments();
    }
}

void NetworkImpairments::set_rate_limit(uint64_t rate_bps) {
    rate_limit_bps_ = rate_bps;
    
    if (enabled_) {
        update_impairments();
    }
}

void NetworkImpairments::set_interface(const std::string& interface) {
    if (enabled_) {
        disable_impairments();
    }
    
    interface_ = interface;
    
    if (enabled_) {
        enable_impairments();
    }
}

NetworkImpairments::Statistics NetworkImpairments::get_statistics() const {
    return stats_;
}

void NetworkImpairments::reset_statistics() {
    stats_ = Statistics();
}

bool NetworkImpairments::is_enabled() const {
    return enabled_;
}

// AdvancedImpairments implementation
AdvancedImpairments::AdvancedImpairments() 
    : enabled_(false), interface_("lo") {
}

AdvancedImpairments::~AdvancedImpairments() {
    disable_all_impairments();
}

bool AdvancedImpairments::add_delay_profile(const std::string& name, 
                                           const DelayProfile& profile) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    delay_profiles_[name] = profile;
    std::cout << "Added delay profile: " << name << std::endl;
    return true;
}

bool AdvancedImpairments::add_loss_profile(const std::string& name, 
                                          const LossProfile& profile) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    loss_profiles_[name] = profile;
    std::cout << "Added loss profile: " << name << std::endl;
    return true;
}

bool AdvancedImpairments::add_burst_profile(const std::string& name, 
                                           const BurstProfile& profile) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    burst_profiles_[name] = profile;
    std::cout << "Added burst profile: " << name << std::endl;
    return true;
}

bool AdvancedImpairments::apply_profile(const std::string& profile_name, 
                                       ProfileType type) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    switch (type) {
        case ProfileType::DELAY: {
            auto it = delay_profiles_.find(profile_name);
            if (it != delay_profiles_.end()) {
                return apply_delay_profile(it->second);
            }
            break;
        }
        case ProfileType::LOSS: {
            auto it = loss_profiles_.find(profile_name);
            if (it != loss_profiles_.end()) {
                return apply_loss_profile(it->second);
            }
            break;
        }
        case ProfileType::BURST: {
            auto it = burst_profiles_.find(profile_name);
            if (it != burst_profiles_.end()) {
                return apply_burst_profile(it->second);
            }
            break;
        }
    }
    
    return false;
}

bool AdvancedImpairments::enable_all_impairments() {
    if (enabled_) {
        return true;
    }
    
    // Apply all active profiles
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    for (const auto& pair : delay_profiles_) {
        if (pair.second.active) {
            apply_delay_profile(pair.second);
        }
    }
    
    for (const auto& pair : loss_profiles_) {
        if (pair.second.active) {
            apply_loss_profile(pair.second);
        }
    }
    
    for (const auto& pair : burst_profiles_) {
        if (pair.second.active) {
            apply_burst_profile(pair.second);
        }
    }
    
    enabled_ = true;
    std::cout << "All impairments enabled" << std::endl;
    return true;
}

bool AdvancedImpairments::disable_all_impairments() {
    if (!enabled_) {
        return true;
    }
    
    // Remove all qdiscs
    std::stringstream cmd;
    cmd << "tc qdisc del dev " << interface_ << " root";
    
    int result = system(cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to disable impairments" << std::endl;
        return false;
    }
    
    enabled_ = false;
    std::cout << "All impairments disabled" << std::endl;
    return true;
}

bool AdvancedImpairments::apply_delay_profile(const DelayProfile& profile) {
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " root netem";
    
    if (profile.base_delay_ms > 0) {
        cmd << " delay " << profile.base_delay_ms << "ms";
        
        if (profile.jitter_ms > 0) {
            cmd << " " << profile.jitter_ms << "ms";
        }
        
        if (profile.distribution != DelayDistribution::UNIFORM) {
            switch (profile.distribution) {
                case DelayDistribution::NORMAL:
                    cmd << " distribution normal";
                    break;
                case DelayDistribution::PARETO:
                    cmd << " distribution pareto";
                    break;
                case DelayDistribution::PARETONORMAL:
                    cmd << " distribution paretonormal";
                    break;
                default:
                    break;
            }
        }
    }
    
    int result = system(cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to apply delay profile: " << cmd.str() << std::endl;
        return false;
    }
    
    std::cout << "Applied delay profile: " << cmd.str() << std::endl;
    return true;
}

bool AdvancedImpairments::apply_loss_profile(const LossProfile& profile) {
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " root netem";
    
    if (profile.loss_percent > 0.0) {
        cmd << " loss " << profile.loss_percent << "%";
        
        if (profile.correlation > 0.0) {
            cmd << " " << profile.correlation << "%";
        }
        
        if (profile.distribution != LossDistribution::RANDOM) {
            switch (profile.distribution) {
                case LossDistribution::STATE:
                    cmd << " state";
                    break;
                case LossDistribution::GEOMETRIC:
                    cmd << " geometric";
                    break;
                default:
                    break;
            }
        }
    }
    
    int result = system(cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to apply loss profile: " << cmd.str() << std::endl;
        return false;
    }
    
    std::cout << "Applied loss profile: " << cmd.str() << std::endl;
    return true;
}

bool AdvancedImpairments::apply_burst_profile(const BurstProfile& profile) {
    std::stringstream cmd;
    cmd << "tc qdisc add dev " << interface_ << " root netem";
    
    if (profile.rate_limit_bps > 0) {
        cmd << " rate " << (profile.rate_limit_bps / 1000) << "kbit";
        
        if (profile.burst_size > 0) {
            cmd << " burst " << profile.burst_size;
        }
        
        if (profile.latency_ms > 0) {
            cmd << " latency " << profile.latency_ms << "ms";
        }
    }
    
    int result = system(cmd.str().c_str());
    if (result != 0) {
        std::cerr << "Failed to apply burst profile: " << cmd.str() << std::endl;
        return false;
    }
    
    std::cout << "Applied burst profile: " << cmd.str() << std::endl;
    return true;
}

void AdvancedImpairments::set_profile_active(const std::string& profile_name, 
                                            ProfileType type, 
                                            bool active) {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    switch (type) {
        case ProfileType::DELAY: {
            auto it = delay_profiles_.find(profile_name);
            if (it != delay_profiles_.end()) {
                it->second.active = active;
            }
            break;
        }
        case ProfileType::LOSS: {
            auto it = loss_profiles_.find(profile_name);
            if (it != loss_profiles_.end()) {
                it->second.active = active;
            }
            break;
        }
        case ProfileType::BURST: {
            auto it = burst_profiles_.find(profile_name);
            if (it != burst_profiles_.end()) {
                it->second.active = active;
            }
            break;
        }
    }
}

std::vector<std::string> AdvancedImpairments::get_profile_names(ProfileType type) const {
    std::lock_guard<std::mutex> lock(profiles_mutex_);
    
    std::vector<std::string> names;
    
    switch (type) {
        case ProfileType::DELAY:
            for (const auto& pair : delay_profiles_) {
                names.push_back(pair.first);
            }
            break;
        case ProfileType::LOSS:
            for (const auto& pair : loss_profiles_) {
                names.push_back(pair.first);
            }
            break;
        case ProfileType::BURST:
            for (const auto& pair : burst_profiles_) {
                names.push_back(pair.first);
            }
            break;
    }
    
    return names;
}

AdvancedImpairments::Statistics AdvancedImpairments::get_statistics() const {
    return stats_;
}

void AdvancedImpairments::reset_statistics() {
    stats_ = Statistics();
}

// ImpairmentSimulator implementation
ImpairmentSimulator::ImpairmentSimulator() 
    : running_(false), simulation_time_(0), packet_count_(0) {
}

ImpairmentSimulator::~ImpairmentSimulator() {
    stop_simulation();
}

bool ImpairmentSimulator::start_simulation(const SimulationConfig& config) {
    if (running_) {
        return false;
    }
    
    config_ = config;
    running_ = true;
    simulation_time_ = 0;
    packet_count_ = 0;
    
    // Start simulation thread
    simulation_thread_ = std::thread(&ImpairmentSimulator::simulation_loop, this);
    
    std::cout << "Impairment simulation started" << std::endl;
    return true;
}

bool ImpairmentSimulator::stop_simulation() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    
    if (simulation_thread_.joinable()) {
        simulation_thread_.join();
    }
    
    std::cout << "Impairment simulation stopped" << std::endl;
    return true;
}

void ImpairmentSimulator::simulation_loop() {
    auto start_time = std::chrono::steady_clock::now();
    
    while (running_) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - start_time).count();
        
        simulation_time_ = elapsed;
        
        // Apply time-based impairments
        apply_time_based_impairments(elapsed);
        
        // Generate test packets
        generate_test_packets();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ImpairmentSimulator::apply_time_based_impairments(uint64_t elapsed_ms) {
    // Apply impairments based on simulation time
    for (const auto& event : config_.events) {
        if (elapsed_ms >= event.start_time_ms && elapsed_ms < event.end_time_ms) {
            // Apply event impairments
            switch (event.type) {
                case EventType::DELAY_SPIKE:
                    // Apply delay spike
                    break;
                case EventType::LOSS_BURST:
                    // Apply loss burst
                    break;
                case EventType::BANDWIDTH_LIMIT:
                    // Apply bandwidth limit
                    break;
            }
        }
    }
}

void ImpairmentSimulator::generate_test_packets() {
    // Generate test packets for simulation
    if (packet_count_ < config_.max_packets) {
        Packet packet;
        packet.size = 64 + (rand() % 1400); // Random packet size
        packet.timestamp = std::chrono::steady_clock::now();
        
        // Apply impairments to packet
        apply_packet_impairments(packet);
        
        packet_count_++;
    }
}

void ImpairmentSimulator::apply_packet_impairments(Packet& packet) {
    // Apply various impairments to the packet
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    
    // Apply loss
    if (dis(gen) < config_.base_loss_percent) {
        stats_.packets_lost++;
        return;
    }
    
    // Apply delay
    if (config_.base_delay_ms > 0) {
        packet.delay_ms = config_.base_delay_ms + (rand() % config_.jitter_ms);
    }
    
    // Apply duplication
    if (dis(gen) < config_.duplicate_percent) {
        stats_.packets_duplicated++;
    }
    
    // Apply corruption
    if (dis(gen) < config_.corruption_percent) {
        stats_.packets_corrupted++;
        // Corrupt packet data
        for (size_t i = 0; i < packet.size && i < 64; ++i) {
            packet.data[i] ^= 0xFF;
        }
    }
    
    stats_.packets_processed++;
}

ImpairmentSimulator::Statistics ImpairmentSimulator::get_statistics() const {
    return stats_;
}

void ImpairmentSimulator::reset_statistics() {
    stats_ = Statistics();
}

} // namespace RouterSim