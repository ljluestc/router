#include "traffic_shaping.h"
#include <iostream>

namespace router_sim {

TrafficShapingManager::TrafficShapingManager() {
}

TrafficShapingManager::~TrafficShapingManager() {
    stop();
}

bool TrafficShapingManager::initialize(const std::map<std::string, std::string>& config) {
    // Parse configuration and initialize default shapers if needed
    auto it = config.find("default_token_bucket");
    if (it != config.end() && it->second == "true") {
        auto token_bucket = std::make_shared<TokenBucketShaper>();
        std::map<std::string, std::string> tb_config;
        tb_config["rate_bps"] = "1000000"; // 1 Mbps
        tb_config["burst_size"] = "10000"; // 10 KB
        token_bucket->initialize(tb_config);
        add_shaper("default_token_bucket", token_bucket);
    }
    
    auto it2 = config.find("default_wfq");
    if (it2 != config.end() && it2->second == "true") {
        auto wfq = std::make_shared<WFQShaper>();
        std::map<std::string, std::string> wfq_config;
        wfq_config["total_bandwidth"] = "10000000"; // 10 Mbps
        wfq_config["max_flows"] = "1000";
        wfq->initialize(wfq_config);
        add_shaper("default_wfq", wfq);
    }
    
    return true;
}

bool TrafficShapingManager::start() {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    bool all_started = true;
    for (auto& pair : shapers_) {
        if (!pair.second->start()) {
            std::cerr << "Failed to start shaper: " << pair.first << std::endl;
            all_started = false;
        }
    }
    
    return all_started;
}

bool TrafficShapingManager::stop() {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    bool all_stopped = true;
    for (auto& pair : shapers_) {
        if (!pair.second->stop()) {
            std::cerr << "Failed to stop shaper: " << pair.first << std::endl;
            all_stopped = false;
        }
    }
    
    return all_stopped;
}

bool TrafficShapingManager::is_running() const {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    for (const auto& pair : shapers_) {
        if (pair.second->is_running()) {
            return true;
        }
    }
    
    return false;
}

bool TrafficShapingManager::add_shaper(const std::string& name, std::shared_ptr<TrafficShaper> shaper) {
    if (!shaper) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    shapers_[name] = shaper;
    
    // Set up callbacks
    shaper->set_packet_dropped_callback([this, name](const PacketInfo& packet) {
        if (packet_dropped_callback_) {
            packet_dropped_callback_(packet, name);
        }
    });
    
    shaper->set_queue_full_callback([this, name]() {
        if (queue_full_callback_) {
            queue_full_callback_(name);
        }
    });
    
    return true;
}

bool TrafficShapingManager::remove_shaper(const std::string& name) {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(name);
    if (it != shapers_.end()) {
        it->second->stop();
        shapers_.erase(it);
        return true;
    }
    
    return false;
}

std::shared_ptr<TrafficShaper> TrafficShapingManager::get_shaper(const std::string& name) const {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    auto it = shapers_.find(name);
    if (it != shapers_.end()) {
        return it->second;
    }
    
    return nullptr;
}

std::vector<std::string> TrafficShapingManager::get_shaper_names() const {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    std::vector<std::string> names;
    for (const auto& pair : shapers_) {
        names.push_back(pair.first);
    }
    
    return names;
}

bool TrafficShapingManager::process_packet(const PacketInfo& packet, const std::string& shaper_name) {
    auto shaper = get_shaper(shaper_name);
    if (!shaper) {
        return false;
    }
    
    return shaper->enqueue_packet(packet);
}

bool TrafficShapingManager::get_processed_packet(PacketInfo& packet, const std::string& shaper_name, int timeout_ms) {
    auto shaper = get_shaper(shaper_name);
    if (!shaper) {
        return false;
    }
    
    return shaper->dequeue_packet(packet, timeout_ms);
}

bool TrafficShapingManager::configure_shaper(const std::string& name, const std::map<std::string, std::string>& config) {
    auto shaper = get_shaper(name);
    if (!shaper) {
        return false;
    }
    
    return shaper->initialize(config);
}

bool TrafficShapingManager::add_traffic_class(const std::string& shaper_name, const TrafficClass& traffic_class) {
    auto shaper = get_shaper(shaper_name);
    if (!shaper) {
        return false;
    }
    
    return shaper->add_traffic_class(traffic_class);
}

bool TrafficShapingManager::remove_traffic_class(const std::string& shaper_name, int class_id) {
    auto shaper = get_shaper(shaper_name);
    if (!shaper) {
        return false;
    }
    
    return shaper->remove_traffic_class(class_id);
}

std::map<std::string, ShapingStatistics> TrafficShapingManager::get_all_statistics() const {
    std::lock_guard<std::mutex> lock(shapers_mutex_);
    
    std::map<std::string, ShapingStatistics> stats;
    for (const auto& pair : shapers_) {
        stats[pair.first] = pair.second->get_statistics();
    }
    
    return stats;
}

ShapingStatistics TrafficShapingManager::get_shaper_statistics(const std::string& name) const {
    auto shaper = get_shaper(name);
    if (!shaper) {
        return ShapingStatistics{};
    }
    
    return shaper->get_statistics();
}

void TrafficShapingManager::set_packet_dropped_callback(std::function<void(const PacketInfo&, const std::string&)> callback) {
    packet_dropped_callback_ = callback;
}

void TrafficShapingManager::set_queue_full_callback(std::function<void(const std::string&)> callback) {
    queue_full_callback_ = callback;
}

} // namespace router_sim
