#include "protocol_interface.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <vector>
#include <mutex>

namespace router_sim {

class ISISProtocol : public ProtocolInterface {
public:
    ISISProtocol() : running_(false), system_id_(""), area_id_(""), level_(2) {}
    
    virtual ~ISISProtocol() {
        stop();
    }
    
    // Core protocol operations
    virtual bool initialize(const std::map<std::string, std::string>& config) override {
        std::lock_guard<std::mutex> lock(config_mutex_);
        
        auto it = config.find("system_id");
        if (it != config.end()) {
            system_id_ = it->second;
        }
        
        it = config.find("area_id");
        if (it != config.end()) {
            area_id_ = it->second;
        }
        
        it = config.find("level");
        if (it != config.end()) {
            level_ = std::stoi(it->second);
        }
        
        it = config.find("hello_interval");
        if (it != config.end()) {
            hello_interval_ = std::stoi(it->second);
        }
        
        it = config.find("hold_time");
        if (it != config.end()) {
            hold_time_ = std::stoi(it->second);
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
        
        // Start ISIS daemon thread
        daemon_thread_ = std::thread(&ISISProtocol::daemon_loop, this);
        
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
        neighbor.protocol = "ISIS";
        neighbor.state = "Down";
        neighbor.last_hello = std::chrono::steady_clock::now();
        neighbor.hold_time = hold_time_;
        
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
    
    std::string system_id_;
    std::string area_id_;
    int level_;
    int hello_interval_;
    int hold_time_;
    
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
            
            // Simulate ISIS neighbor state machine
            if (neighbor.state == "Down") {
                neighbor.state = "Init";
            } else if (neighbor.state == "Init") {
                neighbor.state = "Up";
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
            
            if (elapsed > hold_time_) {
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

// Factory function to create ISIS protocol instance
std::unique_ptr<ProtocolInterface> create_isis_protocol() {
    return std::make_unique<ISISProtocol>();
}

} // namespace router_sim
