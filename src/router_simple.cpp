#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

namespace RouterSim {

// Simple packet structure
struct Packet {
    uint64_t id;
    uint32_t size;
    std::string src_ip;
    std::string dst_ip;
    uint8_t protocol;
    std::chrono::steady_clock::time_point timestamp;
    
    Packet() : id(0), size(0), protocol(0) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Simple route structure
struct Route {
    std::string destination;
    std::string next_hop;
    uint32_t metric;
    std::string protocol;
    
    Route() : metric(0) {}
};

// Simple router core
class SimpleRouter {
public:
    SimpleRouter() : running_(false) {}
    
    bool initialize() {
        std::cout << "Initializing simple router..." << std::endl;
        return true;
    }
    
    bool start() {
        if (running_) {
            return true;
        }
        
        std::cout << "Starting router..." << std::endl;
        running_ = true;
        return true;
    }
    
    bool stop() {
        if (!running_) {
            return true;
        }
        
        std::cout << "Stopping router..." << std::endl;
        running_ = false;
        return true;
    }
    
    bool is_running() const {
        return running_;
    }
    
    bool add_route(const std::string& destination, const std::string& next_hop, uint32_t metric = 1) {
        std::lock_guard<std::mutex> lock(routes_mutex_);
        
        Route route;
        route.destination = destination;
        route.next_hop = next_hop;
        route.metric = metric;
        route.protocol = "STATIC";
        
        routes_[destination] = route;
        std::cout << "Added route: " << destination << " -> " << next_hop << std::endl;
        return true;
    }
    
    std::vector<Route> get_routes() const {
        std::lock_guard<std::mutex> lock(routes_mutex_);
        std::vector<Route> result;
        
        for (const auto& [dest, route] : routes_) {
            result.push_back(route);
        }
        
        return result;
    }
    
    bool process_packet(const Packet& packet) {
        if (!running_) {
            return false;
        }
        
        // Simple packet processing
        std::cout << "Processing packet: " << packet.src_ip << " -> " << packet.dst_ip 
                  << " (size: " << packet.size << ")" << std::endl;
        
        // Look up route
        std::string next_hop = find_next_hop(packet.dst_ip);
        if (!next_hop.empty()) {
            std::cout << "  Next hop: " << next_hop << std::endl;
            return true;
        } else {
            std::cout << "  No route found" << std::endl;
            return false;
        }
    }
    
    void print_routes() const {
        auto routes = get_routes();
        std::cout << "\nRouting Table:" << std::endl;
        std::cout << "Destination\tNext Hop\tMetric\tProtocol" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        for (const auto& route : routes) {
            std::cout << route.destination << "\t\t" << route.next_hop 
                      << "\t\t" << route.metric << "\t" << route.protocol << std::endl;
        }
    }

private:
    std::atomic<bool> running_;
    std::map<std::string, Route> routes_;
    mutable std::mutex routes_mutex_;
    
    std::string find_next_hop(const std::string& destination) const {
        std::lock_guard<std::mutex> lock(routes_mutex_);
        
        // Simple longest prefix match
        std::string best_match;
        size_t best_length = 0;
        
        for (const auto& [dest, route] : routes_) {
            if (destination.find(dest) == 0 && dest.length() > best_length) {
                best_match = route.next_hop;
                best_length = dest.length();
            }
        }
        
        return best_match;
    }
};

// Simple traffic shaper
class SimpleTrafficShaper {
public:
    SimpleTrafficShaper() : enabled_(false), rate_limit_(1000000) {} // 1 Mbps
    
    bool initialize() {
        std::cout << "Initializing traffic shaper..." << std::endl;
        return true;
    }
    
    bool process_packet(const Packet& packet) {
        if (!enabled_) {
            return true;
        }
        
        // Simple rate limiting
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_packet_time_);
        
        if (elapsed.count() > 0) {
            uint64_t allowed_bytes = (rate_limit_ * elapsed.count()) / 1000;
            if (packet.size <= allowed_bytes) {
                last_packet_time_ = now;
                return true;
            }
        }
        
        return false;
    }
    
    void set_enabled(bool enabled) {
        enabled_ = enabled;
    }
    
    void set_rate_limit(uint64_t rate) {
        rate_limit_ = rate;
    }

private:
    bool enabled_;
    uint64_t rate_limit_;
    std::chrono::steady_clock::time_point last_packet_time_;
};

} // namespace RouterSim

// Main function
int main(int argc, char* argv[]) {
    std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                Simple Router Simulator                      ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "║  Features:                                                   ║" << std::endl;
    std::cout << "║  • Basic routing table management                            ║" << std::endl;
    std::cout << "║  • Packet processing                                         ║" << std::endl;
    std::cout << "║  • Simple traffic shaping                                    ║" << std::endl;
    std::cout << "║  • Multi-threaded operation                                  ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;
    
    RouterSim::SimpleRouter router;
    RouterSim::SimpleTrafficShaper shaper;
    
    // Initialize components
    if (!router.initialize()) {
        std::cerr << "Failed to initialize router" << std::endl;
        return 1;
    }
    
    if (!shaper.initialize()) {
        std::cerr << "Failed to initialize traffic shaper" << std::endl;
        return 1;
    }
    
    // Start router
    if (!router.start()) {
        std::cerr << "Failed to start router" << std::endl;
        return 1;
    }
    
    // Add some sample routes
    router.add_route("192.168.1.0/24", "192.168.1.1", 1);
    router.add_route("10.0.0.0/8", "10.0.0.1", 2);
    router.add_route("0.0.0.0/0", "192.168.1.254", 10); // Default route
    
    // Print routing table
    router.print_routes();
    
    // Process some sample packets
    std::cout << "\nProcessing sample packets:" << std::endl;
    
    RouterSim::Packet packet1;
    packet1.src_ip = "192.168.1.10";
    packet1.dst_ip = "192.168.1.20";
    packet1.size = 1500;
    packet1.protocol = 6; // TCP
    
    RouterSim::Packet packet2;
    packet2.src_ip = "192.168.1.10";
    packet2.dst_ip = "8.8.8.8";
    packet2.size = 64;
    packet2.protocol = 1; // ICMP
    
    router.process_packet(packet1);
    router.process_packet(packet2);
    
    // Test traffic shaping
    std::cout << "\nTesting traffic shaping:" << std::endl;
    shaper.set_enabled(true);
    shaper.set_rate_limit(1000); // 1 Kbps for testing
    
    for (int i = 0; i < 5; ++i) {
        RouterSim::Packet test_packet;
        test_packet.src_ip = "192.168.1.10";
        test_packet.dst_ip = "192.168.1.20";
        test_packet.size = 1000;
        test_packet.protocol = 6;
        
        bool allowed = shaper.process_packet(test_packet);
        std::cout << "Packet " << (i + 1) << ": " << (allowed ? "ALLOWED" : "DROPPED") << std::endl;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop router
    router.stop();
    
    std::cout << "\nRouter simulation completed." << std::endl;
    return 0;
}
