#include "traffic_shaping.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace RouterSim {

// TokenBucket implementation
TokenBucket::TokenBucket(const TokenBucketConfig& config) 
    : config_(config), tokens_(config.capacity), last_update_(std::chrono::steady_clock::now()) {
}

TokenBucket::~TokenBucket() = default;

bool TokenBucket::consume_tokens(uint64_t bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Add tokens based on elapsed time
    add_tokens();
    
    if (tokens_ >= bytes) {
        tokens_ -= bytes;
        return true;
    }
    
    return false;
}

void TokenBucket::add_tokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_);
    
    if (elapsed.count() > 0) {
        uint64_t tokens_to_add = (config_.rate * elapsed.count()) / 1000;
        tokens_ = std::min(config_.capacity, tokens_ + tokens_to_add);
        last_update_ = now;
    }
}

uint64_t TokenBucket::get_available_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Add tokens based on elapsed time
    const_cast<TokenBucket*>(this)->add_tokens();
    
    return tokens_;
}

bool TokenBucket::is_available(uint64_t bytes) const {
    return get_available_tokens() >= bytes;
}

void TokenBucket::update_config(const TokenBucketConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    tokens_ = config.capacity;
    last_update_ = std::chrono::steady_clock::now();
}

TokenBucketConfig TokenBucket::get_config() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

// WeightedFairQueue implementation
WeightedFairQueue::WeightedFairQueue() : virtual_time_(0) {
}

WeightedFairQueue::~WeightedFairQueue() = default;

bool WeightedFairQueue::initialize(const std::vector<WFQClass>& classes) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    classes_ = classes;
    queues_.clear();
    
    // Initialize queues for each class
    for (const auto& wfq_class : classes_) {
        queues_[wfq_class.class_id] = std::queue<QueueItem>();
    }
    
    return true;
}

bool WeightedFairQueue::enqueue_packet(const PacketInfo& packet, uint8_t class_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if class exists
    if (queues_.find(class_id) == queues_.end()) {
        return false;
    }
    
    // Create queue item
    QueueItem item;
    item.packet = packet;
    item.class_id = class_id;
    item.enqueue_time = std::chrono::steady_clock::now();
    item.virtual_finish_time = calculate_virtual_finish_time(packet, class_id);
    
    // Add to queue
    queues_[class_id].push(item);
    
    return true;
}

bool WeightedFairQueue::dequeue_packet(PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    QueueItem item;
    if (select_next_packet(item)) {
        packet = item.packet;
        return true;
    }
    
    return false;
}

bool WeightedFairQueue::is_empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [class_id, queue] : queues_) {
        if (!queue.empty()) {
            return false;
        }
    }
    
    return true;
}

size_t WeightedFairQueue::queue_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total_size = 0;
    for (const auto& [class_id, queue] : queues_) {
        total_size += queue.size();
    }
    
    return total_size;
}

size_t WeightedFairQueue::queue_size(uint8_t class_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = queues_.find(class_id);
    if (it != queues_.end()) {
        return it->second.size();
    }
    
    return 0;
}

bool WeightedFairQueue::add_class(const WFQClass& wfq_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if class already exists
    for (const auto& existing_class : classes_) {
        if (existing_class.class_id == wfq_class.class_id) {
            return false;
        }
    }
    
    classes_.push_back(wfq_class);
    queues_[wfq_class.class_id] = std::queue<QueueItem>();
    
    return true;
}

bool WeightedFairQueue::remove_class(uint8_t class_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Remove from classes vector
    auto it = std::find_if(classes_.begin(), classes_.end(),
                          [class_id](const WFQClass& c) { return c.class_id == class_id; });
    
    if (it != classes_.end()) {
        classes_.erase(it);
    }
    
    // Remove from queues
    queues_.erase(class_id);
    
    return true;
}

bool WeightedFairQueue::update_class(const WFQClass& wfq_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find and update class
    for (auto& existing_class : classes_) {
        if (existing_class.class_id == wfq_class.class_id) {
            existing_class = wfq_class;
            return true;
        }
    }
    
    return false;
}

std::vector<WFQClass> WeightedFairQueue::get_classes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return classes_;
}

void WeightedFairQueue::set_classifier(std::function<uint8_t(const PacketInfo&)> classifier) {
    std::lock_guard<std::mutex> lock(mutex_);
    classifier_ = classifier;
}

uint8_t WeightedFairQueue::classify_packet(const PacketInfo& packet) const {
    if (classifier_) {
        return classifier_(packet);
    }
    
    // Default classification based on DSCP
    if (packet.dscp >= 48) {
        return 1; // High priority
    } else if (packet.dscp >= 32) {
        return 2; // Medium priority
    } else {
        return 3; // Low priority
    }
}

uint64_t WeightedFairQueue::calculate_virtual_finish_time(const PacketInfo& packet, uint8_t class_id) const {
    // Find class weight
    uint32_t weight = 1;
    for (const auto& wfq_class : classes_) {
        if (wfq_class.class_id == class_id) {
            weight = wfq_class.weight;
            break;
        }
    }
    
    // Calculate virtual finish time
    uint64_t virtual_finish_time = virtual_time_ + (packet.size * 8) / weight;
    
    return virtual_finish_time;
}

bool WeightedFairQueue::select_next_packet(QueueItem& item) {
    // Find the packet with the smallest virtual finish time
    uint64_t min_finish_time = UINT64_MAX;
    uint8_t selected_class = 0;
    bool found = false;
    
    for (const auto& [class_id, queue] : queues_) {
        if (!queue.empty()) {
            const QueueItem& front_item = queue.front();
            if (front_item.virtual_finish_time < min_finish_time) {
                min_finish_time = front_item.virtual_finish_time;
                selected_class = class_id;
                found = true;
            }
        }
    }
    
    if (found) {
        item = queues_[selected_class].front();
        queues_[selected_class].pop();
        virtual_time_ = std::max(virtual_time_, item.virtual_finish_time);
        return true;
    }
    
    return false;
}

// TrafficShaper implementation
TrafficShaper::TrafficShaper() 
    : algorithm_(ShapingAlgorithm::TOKEN_BUCKET), running_(false), initialized_(false), stop_processing_(false) {
    stats_.reset();
}

TrafficShaper::~TrafficShaper() {
    stop();
}

bool TrafficShaper::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Initialize token bucket with default config
    TokenBucketConfig default_config;
    default_config.capacity = 1000000;  // 1MB
    default_config.rate = 100000;       // 100KB/s
    default_config.burst_size = 500000; // 500KB
    default_config.allow_burst = true;
    
    token_bucket_ = std::make_unique<TokenBucket>(default_config);
    
    // Initialize WFQ with default classes
    std::vector<WFQClass> default_classes;
    
    WFQClass high_priority;
    high_priority.class_id = 1;
    high_priority.weight = 10;
    high_priority.min_bandwidth = 1000000; // 1MB/s
    high_priority.max_bandwidth = 10000000; // 10MB/s
    high_priority.name = "High Priority";
    high_priority.is_active = true;
    default_classes.push_back(high_priority);
    
    WFQClass medium_priority;
    medium_priority.class_id = 2;
    medium_priority.weight = 5;
    medium_priority.min_bandwidth = 500000; // 500KB/s
    medium_priority.max_bandwidth = 5000000; // 5MB/s
    medium_priority.name = "Medium Priority";
    medium_priority.is_active = true;
    default_classes.push_back(medium_priority);
    
    WFQClass low_priority;
    low_priority.class_id = 3;
    low_priority.weight = 1;
    low_priority.min_bandwidth = 100000; // 100KB/s
    low_priority.max_bandwidth = 1000000; // 1MB/s
    low_priority.name = "Low Priority";
    low_priority.is_active = true;
    default_classes.push_back(low_priority);
    
    wfq_ = std::make_unique<WeightedFairQueue>();
    wfq_->initialize(default_classes);
    
    initialized_ = true;
    return true;
}

bool TrafficShaper::start() {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    if (running_) {
        return true;
    }
    
    stop_processing_ = false;
    running_ = true;
    
    // Start processing thread
    processing_thread_ = std::thread(&TrafficShaper::processing_loop, this);
    
    return true;
}

bool TrafficShaper::stop() {
    if (!running_) {
        return true;
    }
    
    stop_processing_ = true;
    running_ = false;
    
    // Wait for processing thread to finish
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }
    
    return true;
}

bool TrafficShaper::is_running() const {
    return running_;
}

bool TrafficShaper::configure_token_bucket(const TokenBucketConfig& config) {
    if (!token_bucket_) {
        return false;
    }
    
    token_bucket_->update_config(config);
    return true;
}

bool TrafficShaper::configure_wfq(const std::vector<WFQClass>& classes) {
    if (!wfq_) {
        return false;
    }
    
    return wfq_->initialize(classes);
}

bool TrafficShaper::set_shaping_algorithm(ShapingAlgorithm algorithm) {
    algorithm_ = algorithm;
    return true;
}

bool TrafficShaper::process_packet(const PacketInfo& packet) {
    if (!running_) {
        return false;
    }
    
    return process_packet_internal(packet);
}

bool TrafficShaper::process_packet_async(const PacketInfo& packet) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(queue_mutex_);
    packet_queue_.push(packet);
    
    return true;
}

bool TrafficShaper::enqueue_packet(const PacketInfo& packet) {
    return process_packet_async(packet);
}

bool TrafficShaper::dequeue_packet(PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    if (packet_queue_.empty()) {
        return false;
    }
    
    packet = packet_queue_.front();
    packet_queue_.pop();
    
    return true;
}

size_t TrafficShaper::get_queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return packet_queue_.size();
}

bool TrafficShaper::is_queue_empty() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return packet_queue_.empty();
}

TrafficStats TrafficShaper::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void TrafficShaper::reset_statistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.reset();
}

void TrafficShaper::set_packet_callback(std::function<void(const PacketInfo&)> callback) {
    packet_callback_ = callback;
}

void TrafficShaper::set_drop_callback(std::function<void(const PacketInfo&, const std::string&)> callback) {
    drop_callback_ = callback;
}

void TrafficShaper::processing_loop() {
    while (!stop_processing_) {
        PacketInfo packet;
        
        if (dequeue_packet(packet)) {
            process_packet_internal(packet);
        } else {
            // No packets to process, sleep briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

bool TrafficShaper::process_packet_internal(const PacketInfo& packet) {
    bool processed = false;
    std::string drop_reason;
    
    switch (algorithm_) {
        case ShapingAlgorithm::TOKEN_BUCKET:
            if (token_bucket_ && token_bucket_->consume_tokens(packet.size)) {
                processed = true;
            } else {
                drop_reason = "Token bucket limit exceeded";
            }
            break;
            
        case ShapingAlgorithm::WEIGHTED_FAIR_QUEUE:
            if (wfq_) {
                uint8_t class_id = wfq_->classify_packet(packet);
                if (wfq_->enqueue_packet(packet, class_id)) {
                    // Try to dequeue and process
                    PacketInfo dequeued_packet;
                    if (wfq_->dequeue_packet(dequeued_packet)) {
                        processed = true;
                    }
                } else {
                    drop_reason = "WFQ queue full";
                }
            }
            break;
            
        case ShapingAlgorithm::PRIORITY_QUEUE:
            // Simple priority queue implementation
            processed = true;
            break;
            
        case ShapingAlgorithm::RATE_LIMITING:
            // Simple rate limiting implementation
            processed = true;
            break;
    }
    
    update_statistics(packet, !processed);
    
    if (processed) {
        notify_packet_processed(packet);
    } else {
        notify_packet_dropped(packet, drop_reason);
    }
    
    return processed;
}

void TrafficShaper::update_statistics(const PacketInfo& packet, bool dropped) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (dropped) {
        stats_.packets_dropped++;
        stats_.bytes_dropped += packet.size;
    } else {
        stats_.packets_processed++;
        stats_.bytes_processed += packet.size;
    }
    
    // Update queue length
    stats_.queue_length = get_queue_size();
    
    // Update throughput
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - stats_.last_update);
    
    if (elapsed.count() > 0) {
        double current_throughput = (stats_.bytes_processed * 1000.0) / elapsed.count();
        stats_.current_throughput_bps = current_throughput;
        
        if (current_throughput > stats_.peak_throughput_bps) {
            stats_.peak_throughput_bps = current_throughput;
        }
        
        stats_.last_update = now;
    }
}

void TrafficShaper::notify_packet_processed(const PacketInfo& packet) {
    if (packet_callback_) {
        packet_callback_(packet);
    }
}

void TrafficShaper::notify_packet_dropped(const PacketInfo& packet, const std::string& reason) {
    if (drop_callback_) {
        drop_callback_(packet, reason);
    }
}

// TrafficShapingManager implementation
TrafficShapingManager::TrafficShapingManager() : running_(false), initialized_(false) {
    global_stats_.reset();
}

TrafficShapingManager::~TrafficShapingManager() {
    stop();
}

bool TrafficShapingManager::initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    return true;
}

bool TrafficShapingManager::start() {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    if (running_) {
        return true;
    }
    
    running_ = true;
    
    // Start all interface shapers
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    for (auto& [interface_name, shaper] : interfaces_) {
        shaper->start();
    }
    
    return true;
}

bool TrafficShapingManager::stop() {
    if (!running_) {
        return true;
    }
    
    // Stop all interface shapers
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    for (auto& [interface_name, shaper] : interfaces_) {
        shaper->stop();
    }
    
    running_ = false;
    return true;
}

bool TrafficShapingManager::add_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    if (interfaces_.find(interface_name) != interfaces_.end()) {
        return false; // Interface already exists
    }
    
    auto shaper = std::make_unique<TrafficShaper>();
    if (!shaper->initialize()) {
        return false;
    }
    
    interfaces_[interface_name] = std::move(shaper);
    return true;
}

bool TrafficShapingManager::remove_interface(const std::string& interface_name) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface_name);
    if (it == interfaces_.end()) {
        return false; // Interface doesn't exist
    }
    
    it->second->stop();
    interfaces_.erase(it);
    return true;
}

bool TrafficShapingManager::configure_interface(const std::string& interface_name, 
                                               ShapingAlgorithm algorithm,
                                               const std::map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface_name);
    if (it == interfaces_.end()) {
        return false; // Interface doesn't exist
    }
    
    auto& shaper = it->second;
    
    // Configure algorithm
    shaper->set_shaping_algorithm(algorithm);
    
    // Configure based on algorithm
    if (algorithm == ShapingAlgorithm::TOKEN_BUCKET) {
        TokenBucketConfig tb_config;
        tb_config.capacity = std::stoull(config.at("capacity"));
        tb_config.rate = std::stoull(config.at("rate"));
        tb_config.burst_size = std::stoull(config.at("burst_size"));
        tb_config.allow_burst = (config.at("allow_burst") == "true");
        
        shaper->configure_token_bucket(tb_config);
    } else if (algorithm == ShapingAlgorithm::WEIGHTED_FAIR_QUEUE) {
        // Parse WFQ classes from config
        std::vector<WFQClass> classes;
        // Implementation would parse classes from config
        shaper->configure_wfq(classes);
    }
    
    return true;
}

bool TrafficShapingManager::process_packet(const std::string& interface_name, const PacketInfo& packet) {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    auto it = interfaces_.find(interface_name);
    if (it == interfaces_.end()) {
        return false; // Interface doesn't exist
    }
    
    return it->second->process_packet(packet);
}

std::map<std::string, TrafficStats> TrafficShapingManager::get_interface_statistics() const {
    std::lock_guard<std::mutex> lock(interfaces_mutex_);
    
    std::map<std::string, TrafficStats> stats;
    for (const auto& [interface_name, shaper] : interfaces_) {
        stats[interface_name] = shaper->get_statistics();
    }
    
    return stats;
}

TrafficStats TrafficShapingManager::get_global_statistics() const {
    std::lock_guard<std::mutex> lock(global_stats_mutex_);
    return global_stats_;
}

bool TrafficShapingManager::load_config(const std::string& config_file) {
    // Implementation would load configuration from file
    return true;
}

bool TrafficShapingManager::save_config(const std::string& config_file) const {
    // Implementation would save configuration to file
    return true;
}

} // namespace RouterSim