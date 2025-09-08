#include "protocol_interface.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <vector>
#include <mutex>

namespace router_sim {

class OSPFProtocol : public ProtocolInterface {
public:
    OSPFProtocol() : running_(false), router_id_(""), area_id_("0.0.0.0") {}
    
    virtual ~OSPFProtocol() {
        stop();
    }
    
    // Core protocol operations
    virtual bool initialize(const std::map<std::string, std::string>& config) override {
        std::lock_guard<std::mutex> lock(config_mutex_);
        
        auto it = config.find("router_id");
        if (it != config.end()) {
            router_id_ = it->second;
        }
        
        it = config.find("area_id");
        if (it != config.end()) {
            area_id_ = it->second;
        }
        
        it = config.find("hello_interval");
        if (it != config.end()) {
            hello_interval_ = std::stoi(it->second);
        }
        
        it = config.find("dead_interval");
        if (it != config.end()) {
            dead_interval_ = std::stoi(it->second);
        }
        
        initialized_ = true;
        return true;
    }
    
    virtual bool start() override {
        std::lock_guard<std::mutex> lock(state_mutex_);
        
        if (!initialized_) {
            return false;
        }
        
        if (running_) {
            return true;
        }
        
        running_ = true;
        
        // Start OSPF daemon thread
        daemon_thread_ = std::thread(&OSPFProtocol::daemon_loop, this);
        
        return true;
    }
    
    virtual bool stop() override {
        std::lock_guard<std::mutex> lock(state_mutex_);
        
        if (!running_) {
            return true;
        }
        
        running_ = false;
        
        if (daemon_thread_.joinable()) {
            daemon_thread_.join();
        }
        
        return true;
    }
    
    virtual bool is_running() const override {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return running_;
    }
    
    // Neighbor management
    virtual bool add_neighbor(const std::string& address, const std::map<std::string, std::string>& config) override {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        NeighborInfo neighbor;
        neighbor.address = address;
        neighbor.protocol = "OSPF";
        neighbor.state = "Init";
        neighbor.last_hello = std::chrono::steady_clock::now();
        neighbor.hold_time = dead_interval_;
        
        // Copy additional attributes
        for (const auto& pair : config) {
            neighbor.attributes[pair.first] = pair.second;
        }
        
        neighbors_[address] = neighbor;
        
        // Notify callback
        if (neighbor_callback_) {
            neighbor_callback_(neighbor, true);
        }
        
        return true;
    }
    
    virtual bool remove_neighbor(const std::string& address) override {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        auto it = neighbors_.find(address);
        if (it == neighbors_.end()) {
            return false;
        }
        
        NeighborInfo neighbor = it->second;
        neighbors_.erase(it);
        
        // Notify callback
        if (neighbor_callback_) {
            neighbor_callback_(neighbor, false);
        }
        
        return true;
    }
    
    virtual std::vector<NeighborInfo> get_neighbors() const override {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        std::vector<NeighborInfo> result;
        for (const auto& pair : neighbors_) {
            result.push_back(pair.second);
        }
        
        return result;
    }
    
    virtual bool is_neighbor_established(const std::string& address) const override {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        auto it = neighbors_.find(address);
        if (it == neighbors_.end()) {
            return false;
        }
        
        return it->second.is_established();
    }
    
    // Route management
    virtual bool advertise_route(const RouteInfo& route) override {
        std::lock_guard<std::mutex> lock(routes_mutex_);
        
        std::string key = route.destination + "/" + std::to_string(route.prefix_length);
        routes_[key] = route;
        
        // Update statistics
        stats_.routes_advertised++;
        stats_.last_update = std::chrono::steady_clock::now();
        
        // Notify callback
        if (route_callback_) {
            route_callback_(route, true);
        }
        
        return true;
    }
    
    virtual bool withdraw_route(const std::string& destination, uint8_t prefix_length) override {
        std::lock_guard<std::mutex> lock(routes_mutex_);
        
        std::string key = destination + "/" + std::to_string(prefix_length);
        auto it = routes_.find(key);
        if (it == routes_.end()) {
            return false;
        }
        
        RouteInfo route = it->second;
        routes_.erase(it);
        
        // Update statistics
        stats_.routes_withdrawn++;
        stats_.last_update = std::chrono::steady_clock::now();
        
        // Notify callback
        if (route_callback_) {
            route_callback_(route, false);
        }
        
        return true;
    }
    
    virtual std::vector<RouteInfo> get_routes() const override {
        std::lock_guard<std::mutex> lock(routes_mutex_);
        
        std::vector<RouteInfo> result;
        for (const auto& pair : routes_) {
            result.push_back(pair.second);
        }
        
        return result;
    }
    
    // Configuration
    virtual bool update_config(const std::map<std::string, std::string>& config) override {
        std::lock_guard<std::mutex> lock(config_mutex_);
        
        for (const auto& pair : config) {
            config_[pair.first] = pair.second;
        }
        
        return true;
    }
    
    virtual std::map<std::string, std::string> get_config() const override {
        std::lock_guard<std::mutex> lock(config_mutex_);
        return config_;
    }
    
    // Statistics
    virtual ProtocolStatistics get_statistics() const override {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return stats_;
    }
    
    // Event callbacks
    virtual void set_route_update_callback(RouteUpdateCallback callback) override {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        route_callback_ = callback;
    }
    
    virtual void set_neighbor_update_callback(NeighborUpdateCallback callback) override {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        neighbor_callback_ = callback;
    }

private:
    mutable std::mutex state_mutex_;
    mutable std::mutex config_mutex_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex routes_mutex_;
    mutable std::mutex stats_mutex_;
    mutable std::mutex callbacks_mutex_;
    
    bool initialized_;
    bool running_;
    std::thread daemon_thread_;
    
    std::string router_id_;
    std::string area_id_;
    int hello_interval_;
    int dead_interval_;
    
    std::map<std::string, std::string> config_;
    std::map<std::string, NeighborInfo> neighbors_;
    std::map<std::string, RouteInfo> routes_;
    ProtocolStatistics stats_;
    
    RouteUpdateCallback route_callback_;
    NeighborUpdateCallback neighbor_callback_;
    
    void daemon_loop() {
        while (running_) {
            // Send hello packets
            send_hello_packets();
            
            // Check neighbor timeouts
            check_neighbor_timeouts();
            
            // Process received packets
            process_received_packets();
            
            // Sleep for hello interval
            std::this_thread::sleep_for(std::chrono::seconds(hello_interval_));
        }
    }
    
    void send_hello_packets() {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        for (auto& pair : neighbors_) {
            NeighborInfo& neighbor = pair.second;
            
            // Update statistics
            stats_.messages_sent++;
            
            // Simulate hello packet processing
            if (neighbor.state == "Init") {
                neighbor.state = "2-Way";
            } else if (neighbor.state == "2-Way") {
                neighbor.state = "ExStart";
            } else if (neighbor.state == "ExStart") {
                neighbor.state = "Exchange";
            } else if (neighbor.state == "Exchange") {
                neighbor.state = "Loading";
            } else if (neighbor.state == "Loading") {
                neighbor.state = "Full";
            }
            
            neighbor.last_hello = std::chrono::steady_clock::now();
        }
    }
    
    void check_neighbor_timeouts() {
        std::lock_guard<std::mutex> lock(neighbors_mutex_);
        
        auto now = std::chrono::steady_clock::now();
        
        for (auto it = neighbors_.begin(); it != neighbors_.end();) {
            NeighborInfo& neighbor = it->second;
            
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - neighbor.last_hello).count();
            
            if (elapsed > dead_interval_) {
                // Neighbor timeout
                NeighborInfo timed_out_neighbor = neighbor;
                it = neighbors_.erase(it);
                
                // Notify callback
                if (neighbor_callback_) {
                    neighbor_callback_(timed_out_neighbor, false);
                }
                
                stats_.neighbor_down_count++;
            } else {
                ++it;
            }
        }
    }
    
    void process_received_packets() {
        // Simulate packet processing
        stats_.messages_received++;
    }
};

// Factory function to create OSPF protocol instance
std::unique_ptr<ProtocolInterface> create_ospf_protocol() {
    return std::make_unique<OSPFProtocol>();
}

} // namespace router_sim
