#include "traffic_shaping.h"
#include "router_sim.h"
#include <iostream>
#include <algorithm>

namespace RouterSim {

TrafficShaper::TrafficShaper()
    : running_(false)
    , total_packets_processed_(0)
    , total_packets_dropped_(0)
    , total_bytes_processed_(0)
    , total_bytes_dropped_(0) {
}

TrafficShaper::~TrafficShaper() {
    stop();
}

bool TrafficShaper::initialize() {
    std::cout << "Initializing Traffic Shaper..." << std::endl;
    return true;
}

bool TrafficShaper::configure_interface(const std::string& interface, const ShapingConfig& config) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto& shaping = interfaces_[interface];
    shaping.config = config;
    
    // Create token bucket if rate limiting is enabled
    if (config.rate_bps > 0) {
        shaping.token_bucket = std::make_unique<TokenBucket>(config.rate_bps, config.burst_size);
    }
    
    // Create WFQ if enabled
    if (config.enable_wfq) {
        shaping.wfq = std::make_unique<WFQ>(config.num_queues, config.weight_base);
    }
    
    // Initialize statistics
    shaping.packets_processed = 0;
    shaping.packets_dropped = 0;
    shaping.bytes_processed = 0;
    shaping.bytes_dropped = 0;
    
    std::cout << "Configured traffic shaping for interface " << interface 
              << " (rate: " << config.rate_bps << " bps, WFQ: " 
              << (config.enable_wfq ? "enabled" : "disabled") << ")" << std::endl;
    
    return true;
}

bool TrafficShaper::clear_interface(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    // Clear packet queue
    while (!it->second.packet_queue.empty()) {
        it->second.packet_queue.pop();
    }
    
    interfaces_.erase(it);
    
    std::cout << "Cleared traffic shaping for interface " << interface << std::endl;
    return true;
}

bool TrafficShaper::enable_wfq(const std::string& interface, uint32_t num_queues) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    auto& shaping = it->second;
    shaping.config.enable_wfq = true;
    shaping.config.num_queues = num_queues;
    shaping.wfq = std::make_unique<WFQ>(num_queues, shaping.config.weight_base);
    
    std::cout << "Enabled WFQ for interface " << interface 
              << " with " << num_queues << " queues" << std::endl;
    
    return true;
}

bool TrafficShaper::disable_wfq(const std::string& interface) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    auto& shaping = it->second;
    shaping.config.enable_wfq = false;
    shaping.wfq.reset();
    
    std::cout << "Disabled WFQ for interface " << interface << std::endl;
    
    return true;
}

bool TrafficShaper::process_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(packet.source_interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    auto& shaping = it->second;
    
    // Apply token bucket rate limiting
    if (shaping.token_bucket && !apply_token_bucket(shaping, packet)) {
        // Packet dropped by token bucket
        shaping.packets_dropped++;
        shaping.bytes_dropped += packet.size;
        total_packets_dropped_++;
        total_bytes_dropped_ += packet.size;
        return false;
    }
    
    // Apply WFQ scheduling
    if (shaping.wfq && !apply_wfq(shaping, packet)) {
        // Packet dropped by WFQ
        shaping.packets_dropped++;
        shaping.bytes_dropped += packet.size;
        total_packets_dropped_++;
        total_bytes_dropped_ += packet.size;
        return false;
    }
    
    // Packet processed successfully
    shaping.packets_processed++;
    shaping.bytes_processed += packet.size;
    total_packets_processed_++;
    total_bytes_processed_ += packet.size;
    
    return true;
}

bool TrafficShaper::enqueue_packet(const Packet& packet) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(packet.source_interface);
    if (it == interfaces_.end()) {
        return false;
    }
    
    auto& shaping = it->second;
    
    // Check queue size limit
    if (shaping.packet_queue.size() >= shaping.config.queue_size) {
        // Queue full, drop packet
        shaping.packets_dropped++;
        shaping.bytes_dropped += packet.size;
        total_packets_dropped_++;
        total_bytes_dropped_ += packet.size;
        return false;
    }
    
    shaping.packet_queue.push(packet);
    return true;
}

bool TrafficShaper::dequeue_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    // Find interface with packets to dequeue
    for (auto& pair : interfaces_) {
        auto& shaping = pair.second;
        
        if (!shaping.packet_queue.empty()) {
            packet = shaping.packet_queue.front();
            shaping.packet_queue.pop();
            return true;
        }
    }
    
    return false;
}

std::map<std::string, uint64_t> TrafficShaper::get_interface_stats(const std::string& interface) const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    std::map<std::string, uint64_t> stats;
    
    auto it = interfaces_.find(interface);
    if (it != interfaces_.end()) {
        const auto& shaping = it->second;
        stats["packets_processed"] = shaping.packets_processed;
        stats["packets_dropped"] = shaping.packets_dropped;
        stats["bytes_processed"] = shaping.bytes_processed;
        stats["bytes_dropped"] = shaping.bytes_dropped;
        stats["queue_size"] = shaping.packet_queue.size();
        stats["queue_capacity"] = shaping.config.queue_size;
        stats["rate_bps"] = shaping.config.rate_bps;
        stats["burst_size"] = shaping.config.burst_size;
        stats["wfq_enabled"] = shaping.config.enable_wfq;
        stats["num_queues"] = shaping.config.num_queues;
    }
    
    return stats;
}

std::map<std::string, uint64_t> TrafficShaper::get_global_stats() const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    std::map<std::string, uint64_t> stats;
    stats["total_packets_processed"] = total_packets_processed_;
    stats["total_packets_dropped"] = total_packets_dropped_;
    stats["total_bytes_processed"] = total_bytes_processed_;
    stats["total_bytes_dropped"] = total_bytes_dropped_;
    stats["num_interfaces"] = interfaces_.size();
    
    return stats;
}

void TrafficShaper::reset_statistics() {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    total_packets_processed_ = 0;
    total_packets_dropped_ = 0;
    total_bytes_processed_ = 0;
    total_bytes_dropped_ = 0;
    
    for (auto& pair : interfaces_) {
        auto& shaping = pair.second;
        shaping.packets_processed = 0;
        shaping.packets_dropped = 0;
        shaping.bytes_processed = 0;
        shaping.bytes_dropped = 0;
    }
}

void TrafficShaper::start() {
    if (running_.load()) {
        return;
    }
    
    running_.store(true);
    processing_thread_ = std::thread(&TrafficShaper::processing_loop, this);
    
    std::cout << "Traffic Shaper started" << std::endl;
}

void TrafficShaper::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }
    
    std::cout << "Traffic Shaper stopped" << std::endl;
}

bool TrafficShaper::is_running() const {
    return running_.load();
}

void TrafficShaper::processing_loop() {
    while (running_.load()) {
        std::lock_guard<std::mutex> lock(interfaces_mutex_);
        
        for (auto& pair : interfaces_) {
            process_interface_packets(pair.first, pair.second);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

bool TrafficShaper::process_interface_packets(const std::string& interface, InterfaceShaping& shaping) {
    // Process packets from the interface queue
    while (!shaping.packet_queue.empty()) {
        Packet packet = shaping.packet_queue.front();
        shaping.packet_queue.pop();
        
        // Apply traffic shaping
        if (!process_packet(packet)) {
            // Packet was dropped
            continue;
        }
        
        // Packet successfully processed
        // In a real implementation, this would forward the packet
    }
    
    return true;
}

uint32_t TrafficShaper::calculate_queue_id(const Packet& packet) const {
    // Simple queue selection based on packet priority
    // In a real implementation, this would use more sophisticated algorithms
    return packet.priority % 8; // Assuming 8 queues max
}

bool TrafficShaper::apply_token_bucket(InterfaceShaping& shaping, Packet& packet) {
    if (!shaping.token_bucket) {
        return true; // No rate limiting
    }
    
    return shaping.token_bucket->consume_tokens(packet.size);
}

bool TrafficShaper::apply_wfq(InterfaceShaping& shaping, Packet& packet) {
    if (!shaping.wfq) {
        return true; // No WFQ scheduling
    }
    
    uint32_t queue_id = calculate_queue_id(packet);
    return shaping.wfq->enqueue_packet(packet, queue_id);
}

} // namespace RouterSim
