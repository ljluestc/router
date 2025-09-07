#include "netem_impairments.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>

using namespace RouterSim;

// Impairment implementation
Impairment::Impairment(ImpairmentType type, double value, double probability)
    : type_(type), value_(value), probability_(probability), rng_(std::random_device{}()),
      dist_(0.0, 1.0) {
}

bool Impairment::apply(Packet& packet) {
    // Check if impairment should be applied based on probability
    if (dist_(rng_) > probability_) {
        return true; // No impairment applied
    }
    
    switch (type_) {
        case ImpairmentType::DELAY:
            return apply_delay(packet);
        case ImpairmentType::JITTER:
            return apply_jitter(packet);
        case ImpairmentType::LOSS:
            return apply_loss(packet);
        case ImpairmentType::DUPLICATE:
            return apply_duplicate(packet);
        case ImpairmentType::CORRUPTION:
            return apply_corruption(packet);
        case ImpairmentType::REORDER:
            return apply_reorder(packet);
        case ImpairmentType::BANDWIDTH_LIMIT:
            return apply_bandwidth_limit(packet);
        case ImpairmentType::PACKET_CORRUPTION:
            return apply_packet_corruption(packet);
        default:
            return true;
    }
}

ImpairmentType Impairment::get_type() const {
    return type_;
}

double Impairment::get_value() const {
    return value_;
}

double Impairment::get_probability() const {
    return probability_;
}

void Impairment::set_value(double value) {
    value_ = value;
}

void Impairment::set_probability(double probability) {
    probability_ = std::max(0.0, std::min(1.0, probability));
}

bool Impairment::apply_delay(Packet& packet) {
    // Add delay by modifying timestamp
    auto delay_ms = static_cast<uint32_t>(value_);
    packet.timestamp += std::chrono::milliseconds(delay_ms);
    return true;
}

bool Impairment::apply_jitter(Packet& packet) {
    // Add jitter by adding random delay
    std::uniform_int_distribution<uint32_t> jitter_dist(0, static_cast<uint32_t>(value_));
    uint32_t jitter_ms = jitter_dist(rng_);
    packet.timestamp += std::chrono::milliseconds(jitter_ms);
    return true;
}

bool Impairment::apply_loss(Packet& packet) {
    // Packet loss - return false to indicate packet should be dropped
    return false;
}

bool Impairment::apply_duplicate(Packet& packet) {
    // Duplicate packet - this would need to be handled by the caller
    // For now, just return true (no modification)
    return true;
}

bool Impairment::apply_corruption(Packet& packet) {
    // Corrupt packet data
    if (packet.data.empty()) {
        return true;
    }
    
    std::uniform_int_distribution<size_t> byte_dist(0, packet.data.size() - 1);
    std::uniform_int_distribution<uint8_t> value_dist(0, 255);
    
    size_t corrupt_byte = byte_dist(rng_);
    packet.data[corrupt_byte] = value_dist(rng_);
    
    return true;
}

bool Impairment::apply_reorder(Packet& packet) {
    // Packet reordering - this would need to be handled by the caller
    // For now, just return true (no modification)
    return true;
}

bool Impairment::apply_bandwidth_limit(Packet& packet) {
    // Bandwidth limiting - this would need to be handled by traffic shaping
    // For now, just return true (no modification)
    return true;
}

bool Impairment::apply_packet_corruption(Packet& packet) {
    // Corrupt packet header or payload
    if (packet.data.size() < 4) {
        return true;
    }
    
    std::uniform_int_distribution<size_t> byte_dist(0, std::min(packet.data.size() - 1, size_t(20)));
    std::uniform_int_distribution<uint8_t> value_dist(0, 255);
    
    size_t corrupt_byte = byte_dist(rng_);
    packet.data[corrupt_byte] = value_dist(rng_);
    
    return true;
}

// NetemImpairments implementation
NetemImpairments::NetemImpairments() : running_(false), rng_(std::random_device{}()), uniform_dist_(0.0, 1.0) {
}

NetemImpairments::~NetemImpairments() {
    stop();
}

bool NetemImpairments::add_interface(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    InterfaceImpairments impairments;
    impairments.tc_applied = false;
    interfaces_[interface] = std::move(impairments);
    
    return true;
}

bool NetemImpairments::remove_interface(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Remove TC rules if applied
    if (it->second.tc_applied) {
        remove_tc_rules(interface);
    }
    
    interfaces_.erase(it);
    return true;
}

bool NetemImpairments::configure_impairments(const std::string& interface, const ImpairmentConfig& config) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Clear existing impairments
    it->second.impairments.clear();
    
    // Add new impairments based on configuration
    if (config.enable_delay) {
        it->second.impairments.push_back(
            std::make_unique<Impairment>(ImpairmentType::DELAY, config.delay_ms, 1.0)
        );
    }
    
    if (config.enable_jitter) {
        it->second.impairments.push_back(
            std::make_unique<Impairment>(ImpairmentType::JITTER, config.jitter_ms, 1.0)
        );
    }
    
    if (config.enable_loss) {
        it->second.impairments.push_back(
            std::make_unique<Impairment>(ImpairmentType::LOSS, config.loss_percent, config.loss_percent / 100.0)
        );
    }
    
    if (config.enable_duplicate) {
        it->second.impairments.push_back(
            std::make_unique<Impairment>(ImpairmentType::DUPLICATE, config.duplicate_percent, config.duplicate_percent / 100.0)
        );
    }
    
    if (config.enable_corruption) {
        it->second.impairments.push_back(
            std::make_unique<Impairment>(ImpairmentType::CORRUPTION, config.corruption_percent, config.corruption_percent / 100.0)
        );
    }
    
    if (config.enable_reorder) {
        it->second.impairments.push_back(
            std::make_unique<Impairment>(ImpairmentType::REORDER, config.reorder_percent, config.reorder_percent / 100.0)
        );
    }
    
    // Apply TC rules if available
    if (is_tc_available()) {
        apply_tc_rules(interface, config);
        it->second.tc_applied = true;
    }
    
    return true;
}

bool NetemImpairments::add_impairment(const std::string& interface, ImpairmentType type, double value, double probability) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    it->second.impairments.push_back(
        std::make_unique<Impairment>(type, value, probability)
    );
    
    return true;
}

bool NetemImpairments::remove_impairment(const std::string& interface, ImpairmentType type) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    auto& impairments = it->second.impairments;
    impairments.erase(
        std::remove_if(impairments.begin(), impairments.end(),
                      [type](const std::unique_ptr<Impairment>& imp) {
                          return imp->get_type() == type;
                      }),
        impairments.end()
    );
    
    return true;
}

bool NetemImpairments::clear_impairments(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    it->second.impairments.clear();
    
    // Remove TC rules if applied
    if (it->second.tc_applied) {
        remove_tc_rules(interface);
        it->second.tc_applied = false;
    }
    
    return true;
}

bool NetemImpairments::apply_tc_rules(const std::string& interface, const ImpairmentConfig& config) {
    // Remove existing rules first
    remove_tc_rules(interface);
    
    std::string command = "tc qdisc add dev " + interface + " root netem";
    std::vector<std::string> parameters;
    
    if (config.enable_delay) {
        parameters.push_back("delay " + std::to_string(config.delay_ms) + "ms");
    }
    
    if (config.enable_jitter) {
        parameters.push_back("jitter " + std::to_string(config.jitter_ms) + "ms");
    }
    
    if (config.enable_loss) {
        parameters.push_back("loss " + std::to_string(config.loss_percent) + "%");
    }
    
    if (config.enable_duplicate) {
        parameters.push_back("duplicate " + std::to_string(config.duplicate_percent) + "%");
    }
    
    if (config.enable_corruption) {
        parameters.push_back("corrupt " + std::to_string(config.corruption_percent) + "%");
    }
    
    if (config.enable_reorder) {
        parameters.push_back("reorder " + std::to_string(config.reorder_percent) + "%");
    }
    
    if (!parameters.empty()) {
        command += " " + std::string(parameters.begin(), parameters.end());
        return execute_tc_command(command);
    }
    
    return true;
}

bool NetemImpairments::remove_tc_rules(const std::string& interface) {
    std::string command = "tc qdisc del dev " + interface + " root";
    return execute_tc_command(command);
}

bool NetemImpairments::is_tc_available() const {
    return system("which tc > /dev/null 2>&1") == 0;
}

bool NetemImpairments::process_packet(const std::string& interface, Packet& packet) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return true; // No impairments configured
    }
    
    bool packet_processed = true;
    
    for (auto& impairment : it->second.impairments) {
        if (!impairment->apply(packet)) {
            packet_processed = false;
            break; // Packet dropped
        }
    }
    
    // Update statistics
    if (packet_processed) {
        it->second.stats["packets_processed"]++;
        it->second.stats["bytes_processed"] += packet.size;
    } else {
        it->second.stats["packets_dropped"]++;
        it->second.stats["bytes_dropped"] += packet.size;
    }
    
    return packet_processed;
}

std::vector<Packet> NetemImpairments::process_packets(const std::string& interface, const std::vector<Packet>& packets) {
    std::vector<Packet> result;
    
    for (const auto& packet : packets) {
        Packet processed_packet = packet;
        if (process_packet(interface, processed_packet)) {
            result.push_back(processed_packet);
        }
    }
    
    return result;
}

std::map<std::string, uint64_t> NetemImpairments::get_impairment_stats(const std::string& interface) const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return {};
    }
    
    return it->second.stats;
}

std::map<std::string, uint64_t> NetemImpairments::get_global_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return global_stats_;
}

void NetemImpairments::reset_statistics() {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    for (auto& pair : interfaces_) {
        pair.second.stats.clear();
    }
    
    global_stats_.clear();
}

void NetemImpairments::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    std::cout << "Netem impairments started\n";
}

void NetemImpairments::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Remove all TC rules
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    for (const auto& pair : interfaces_) {
        if (pair.second.tc_applied) {
            remove_tc_rules(pair.first);
        }
    }
    
    std::cout << "Netem impairments stopped\n";
}

bool NetemImpairments::is_running() const {
    return running_;
}

std::string NetemImpairments::generate_tc_delay_command(const std::string& interface, uint32_t delay_ms) const {
    return "tc qdisc add dev " + interface + " root netem delay " + std::to_string(delay_ms) + "ms";
}

std::string NetemImpairments::generate_tc_jitter_command(const std::string& interface, uint32_t jitter_ms) const {
    return "tc qdisc add dev " + interface + " root netem jitter " + std::to_string(jitter_ms) + "ms";
}

std::string NetemImpairments::generate_tc_loss_command(const std::string& interface, double loss_percent) const {
    return "tc qdisc add dev " + interface + " root netem loss " + std::to_string(loss_percent) + "%";
}

std::string NetemImpairments::generate_tc_duplicate_command(const std::string& interface, double duplicate_percent) const {
    return "tc qdisc add dev " + interface + " root netem duplicate " + std::to_string(duplicate_percent) + "%";
}

std::string NetemImpairments::generate_tc_corruption_command(const std::string& interface, double corruption_percent) const {
    return "tc qdisc add dev " + interface + " root netem corrupt " + std::to_string(corruption_percent) + "%";
}

std::string NetemImpairments::generate_tc_reorder_command(const std::string& interface, double reorder_percent) const {
    return "tc qdisc add dev " + interface + " root netem reorder " + std::to_string(reorder_percent) + "%";
}

bool NetemImpairments::execute_tc_command(const std::string& command) {
    int result = system(command.c_str());
    return result == 0;
}

std::string NetemImpairments::get_tc_output(const std::string& command) {
    std::string output;
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }
        pclose(pipe);
    }
    return output;
}

bool NetemImpairments::should_apply_impairment(const std::string& interface, ImpairmentType type) const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    for (const auto& impairment : it->second.impairments) {
        if (impairment->get_type() == type) {
            return true;
        }
    }
    
    return false;
}

void NetemImpairments::update_statistics(const std::string& interface, ImpairmentType type, bool applied) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::string key = interface + "_" + std::to_string(static_cast<int>(type));
    if (applied) {
        global_stats_[key + "_applied"]++;
    } else {
        global_stats_[key + "_skipped"]++;
    }
}

// ImpairmentSimulator implementation
bool ImpairmentSimulator::simulate_high_latency(Packet& packet, uint32_t base_delay_ms, uint32_t jitter_ms) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> jitter_dist(0, jitter_ms);
    
    uint32_t total_delay = base_delay_ms + jitter_dist(gen);
    packet.timestamp += std::chrono::milliseconds(total_delay);
    
    return true;
}

bool ImpairmentSimulator::simulate_packet_loss(Packet& packet, double loss_rate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    return dist(gen) > loss_rate;
}

bool ImpairmentSimulator::simulate_bandwidth_constraint(Packet& packet, uint32_t max_bandwidth_bps) {
    // This would need to be integrated with traffic shaping
    // For now, just return true
    return true;
}

bool ImpairmentSimulator::simulate_network_congestion(Packet& packet, double congestion_factor) {
    // Simulate congestion by adding delay proportional to congestion factor
    uint32_t delay_ms = static_cast<uint32_t>(congestion_factor * 100);
    packet.timestamp += std::chrono::milliseconds(delay_ms);
    
    return true;
}

bool ImpairmentSimulator::simulate_satellite_link(Packet& packet) {
    // Satellite links have high latency (250-500ms) and some packet loss
    return simulate_high_latency(packet, 300, 50) && simulate_packet_loss(packet, 0.01);
}

bool ImpairmentSimulator::simulate_mobile_network(Packet& packet) {
    // Mobile networks have variable latency and occasional packet loss
    return simulate_high_latency(packet, 50, 100) && simulate_packet_loss(packet, 0.05);
}

bool ImpairmentSimulator::simulate_dsl_connection(Packet& packet) {
    // DSL has moderate latency and low packet loss
    return simulate_high_latency(packet, 20, 10) && simulate_packet_loss(packet, 0.001);
}

bool ImpairmentSimulator::simulate_fiber_connection(Packet& packet) {
    // Fiber has low latency and very low packet loss
    return simulate_high_latency(packet, 5, 2) && simulate_packet_loss(packet, 0.0001);
}

bool ImpairmentSimulator::simulate_poor_connection(Packet& packet) {
    // Poor connection: high latency, high jitter, high packet loss
    return simulate_high_latency(packet, 200, 100) && simulate_packet_loss(packet, 0.1);
}

bool ImpairmentSimulator::simulate_unstable_connection(Packet& packet) {
    // Unstable connection: variable latency and moderate packet loss
    return simulate_high_latency(packet, 100, 150) && simulate_packet_loss(packet, 0.03);
}

bool ImpairmentSimulator::simulate_congested_network(Packet& packet) {
    // Congested network: high latency due to queuing
    return simulate_network_congestion(packet, 2.0);
}
