#include "traffic_shaping.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <functional>

namespace router_sim {

WFQShaper::WFQShaper() 
    : running_(false), scheduling_running_(false) {
}

bool WFQShaper::initialize(const std::map<std::string, std::string>& config) {
    // Parse configuration
    auto it = config.find("total_bandwidth");
    if (it != config.end()) {
        config_.total_bandwidth = std::stoull(it->second);
    }
    
    it = config.find("enable_flow_control");
    if (it != config.end()) {
        config_.enable_flow_control = (it->second == "true" || it->second == "1");
    }
    
    it = config.find("max_flows");
    if (it != config.end()) {
        config_.max_flows = std::stoul(it->second);
    }
    
    // Set defaults if not specified
    if (config_.total_bandwidth == 0) {
        config_.total_bandwidth = 10000000; // 10 Mbps default
    }
    
    return true;
}

bool WFQShaper::start() {
    if (running_) {
        return true;
    }
    
    running_ = true;
    scheduling_running_ = true;
    
    // Start scheduling thread
    scheduling_thread_ = std::thread(&WFQShaper::scheduling_loop, this);
    
    return true;
}

bool WFQShaper::stop() {
    if (!running_) {
        return true;
    }
    
    running_ = false;
    scheduling_running_ = false;
    
    if (scheduling_thread_.joinable()) {
        scheduling_thread_.join();
    }
    
    return true;
}

bool WFQShaper::is_running() const {
    return running_;
}

bool WFQShaper::enqueue_packet(const PacketInfo& packet) {
    if (!running_) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(flows_mutex_);
    
    // Calculate flow ID based on packet characteristics
    int flow_id = calculate_flow_id(packet);
    
    // Check if flow exists
    auto flow_it = flows_.find(flow_id);
    if (flow_it == flows_.end()) {
        // Create new flow
        if (flows_.size() >= config_.max_flows) {
            // Drop packet if too many flows
            if (packet_dropped_callback_) {
                packet_dropped_callback_(packet);
            }
            return false;
        }
        
        FlowInfo new_flow;
        new_flow.flow_id = flow_id;
        new_flow.class_id = packet.traffic_class;
        new_flow.finish_time = 0.0;
        flows_[flow_id] = new_flow;
        flow_it = flows_.find(flow_id);
    }
    
    // Add packet to flow queue
    flow_it->second.packets.push(packet);
    
    // Calculate finish time for this packet
    double finish_time = calculate_finish_time(packet, packet.traffic_class);
    flow_it->second.finish_time = finish_time;
    
    // Update priority queue
    finish_time_queue_.push({finish_time, flow_id});
    
    return true;
}

bool WFQShaper::dequeue_packet(PacketInfo& packet, int timeout_ms) {
    if (!running_) {
        return false;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    while (running_) {
        std::lock_guard<std::mutex> lock(flows_mutex_);
        
        // Find the flow with the smallest finish time
        while (!finish_time_queue_.empty()) {
            auto top = finish_time_queue_.top();
            double finish_time = top.first;
            int flow_id = top.second;
            finish_time_queue_.pop();
            
            auto flow_it = flows_.find(flow_id);
            if (flow_it != flows_.end() && !flow_it->second.packets.empty()) {
                packet = flow_it->second.packets.front();
                flow_it->second.packets.pop();
                
                // Update statistics
                {
                    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
                    statistics_.packets_processed++;
                    statistics_.bytes_processed += packet.size;
                    
                    // Update class statistics
                    auto class_it = class_statistics_.find(packet.traffic_class);
                    if (class_it != class_statistics_.end()) {
                        class_it->second.packets_processed++;
                        class_it->second.bytes_processed += packet.size;
                    }
                }
                
                // If flow is empty, remove it
                if (flow_it->second.packets.empty()) {
                    flows_.erase(flow_it);
                }
                
                return true;
            }
        }
        
        // Check timeout
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        if (elapsed.count() >= timeout_ms) {
            break;
        }
        
        // Small delay before retry
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    return false;
}

size_t WFQShaper::queue_size() const {
    std::lock_guard<std::mutex> lock(flows_mutex_);
    size_t total_size = 0;
    for (const auto& pair : flows_) {
        total_size += pair.second.packets.size();
    }
    return total_size;
}

bool WFQShaper::is_queue_full() const {
    std::lock_guard<std::mutex> lock(flows_mutex_);
    return flows_.size() >= config_.max_flows;
}

bool WFQShaper::add_traffic_class(const TrafficClass& traffic_class) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    traffic_classes_[traffic_class.class_id] = traffic_class;
    
    // Initialize class statistics
    {
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        class_statistics_[traffic_class.class_id] = ShapingStatistics{};
    }
    
    return true;
}

bool WFQShaper::remove_traffic_class(int class_id) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    auto it = traffic_classes_.find(class_id);
    if (it != traffic_classes_.end()) {
        traffic_classes_.erase(it);
        
        // Remove class statistics
        {
            std::lock_guard<std::mutex> stats_lock(stats_mutex_);
            class_statistics_.erase(class_id);
        }
        
        return true;
    }
    return false;
}

bool WFQShaper::update_traffic_class(const TrafficClass& traffic_class) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    auto it = traffic_classes_.find(traffic_class.class_id);
    if (it != traffic_classes_.end()) {
        it->second = traffic_class;
        return true;
    }
    return false;
}

std::vector<TrafficClass> WFQShaper::get_traffic_classes() const {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    std::vector<TrafficClass> classes;
    for (const auto& pair : traffic_classes_) {
        classes.push_back(pair.second);
    }
    return classes;
}

ShapingStatistics WFQShaper::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return statistics_;
}

ShapingStatistics WFQShaper::get_class_statistics(int class_id) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = class_statistics_.find(class_id);
    if (it != class_statistics_.end()) {
        return it->second;
    }
    return ShapingStatistics{};
}

void WFQShaper::set_packet_dropped_callback(std::function<void(const PacketInfo&)> callback) {
    packet_dropped_callback_ = callback;
}

void WFQShaper::set_queue_full_callback(std::function<void()> callback) {
    queue_full_callback_ = callback;
}

bool WFQShaper::set_class_weight(int class_id, double weight) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    auto it = traffic_classes_.find(class_id);
    if (it != traffic_classes_.end()) {
        it->second.weight = weight;
        return true;
    }
    return false;
}

bool WFQShaper::set_class_bandwidth(int class_id, uint64_t bandwidth_bps) {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    auto it = traffic_classes_.find(class_id);
    if (it != traffic_classes_.end()) {
        it->second.bandwidth_bps = bandwidth_bps;
        return true;
    }
    return false;
}

double WFQShaper::calculate_finish_time(const PacketInfo& packet, int class_id) const {
    std::lock_guard<std::mutex> lock(classes_mutex_);
    
    auto it = traffic_classes_.find(class_id);
    if (it == traffic_classes_.end()) {
        return 0.0;
    }
    
    const TrafficClass& traffic_class = it->second;
    
    // Calculate finish time using WFQ formula
    // F = max(A, F_prev) + (packet_size * weight) / (class_bandwidth * weight)
    double current_time = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count() / 1000000.0;
    
    double packet_size_bits = packet.size * 8.0;
    double class_bandwidth = traffic_class.bandwidth_bps;
    double weight = traffic_class.weight;
    
    if (class_bandwidth == 0 || weight == 0) {
        return current_time;
    }
    
    // Find the flow's previous finish time
    double prev_finish_time = 0.0;
    {
        std::lock_guard<std::mutex> flows_lock(flows_mutex_);
        int flow_id = calculate_flow_id(packet);
        auto flow_it = flows_.find(flow_id);
        if (flow_it != flows_.end()) {
            prev_finish_time = flow_it->second.finish_time;
        }
    }
    
    double arrival_time = current_time;
    double finish_time = std::max(arrival_time, prev_finish_time) + 
                        (packet_size_bits * weight) / (class_bandwidth * weight);
    
    return finish_time;
}

void WFQShaper::scheduling_loop() {
    while (scheduling_running_) {
        // The actual scheduling is done in dequeue_packet
        // This loop can be used for periodic maintenance tasks
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool WFQShaper::process_packet(const PacketInfo& packet) {
    // This method can be used for additional packet processing logic
    return true;
}

int WFQShaper::calculate_flow_id(const PacketInfo& packet) const {
    // Simple hash-based flow ID calculation
    // In a real implementation, this would be based on 5-tuple or other flow characteristics
    std::hash<std::string> hasher;
    std::string flow_key = packet.source_interface + ":" + packet.destination_interface + 
                          ":" + std::to_string(packet.traffic_class);
    return static_cast<int>(hasher(flow_key) % 1000000);
}

} // namespace router_sim
