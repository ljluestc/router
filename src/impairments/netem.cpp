#include "traffic_shaping.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

namespace RouterSim {

class NetEmImpairment : public NetworkImpairment {
public:
    NetEmImpairment() : enabled_(false), packet_loss_(0.0), delay_(0.0), jitter_(0.0) {}
    
    virtual ~NetEmImpairment() = default;
    
    virtual bool initialize(const std::map<std::string, std::string>& config) override {
        auto it = config.find("enabled");
        if (it != config.end()) {
            enabled_ = (it->second == "true" || it->second == "1");
        }
        
        it = config.find("packet_loss");
        if (it != config.end()) {
            packet_loss_ = std::stod(it->second);
        }
        
        it = config.find("delay");
        if (it != config.end()) {
            delay_ = std::stod(it->second);
        }
        
        it = config.find("jitter");
        if (it != config.end()) {
            jitter_ = std::stod(it->second);
        }
        
        it = config.find("distribution");
        if (it != config.end()) {
            distribution_ = it->second;
        }
        
        // Initialize random number generator
        rng_.seed(std::chrono::steady_clock::now().time_since_epoch().count());
        
        return true;
    }
    
    virtual bool apply(PacketInfo& packet) override {
        if (!enabled_) {
            return true;
        }
        
        // Apply packet loss
        if (should_drop_packet()) {
            return false; // Drop packet
        }
        
        // Apply delay
        if (delay_ > 0.0) {
            apply_delay(packet);
        }
        
        // Apply jitter
        if (jitter_ > 0.0) {
            apply_jitter(packet);
        }
        
        return true;
    }
    
    virtual bool is_enabled() const override {
        return enabled_;
    }
    
    virtual void set_enabled(bool enabled) override {
        enabled_ = enabled;
    }
    
    virtual std::map<std::string, std::string> get_config() const override {
        return {
            {"enabled", enabled_ ? "true" : "false"},
            {"packet_loss", std::to_string(packet_loss_)},
            {"delay", std::to_string(delay_)},
            {"jitter", std::to_string(jitter_)},
            {"distribution", distribution_}
        };
    }

private:
    bool enabled_;
    double packet_loss_;
    double delay_;
    double jitter_;
    std::string distribution_;
    
    std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_dist_;
    
    bool should_drop_packet() {
        if (packet_loss_ <= 0.0) {
            return false;
        }
        
        double random_value = uniform_dist_(rng_);
        return random_value < packet_loss_;
    }
    
    void apply_delay(PacketInfo& packet) {
        if (delay_ <= 0.0) {
            return;
        }
        
        double actual_delay = delay_;
        
        // Add jitter to delay if specified
        if (jitter_ > 0.0) {
            double jitter_offset = (uniform_dist_(rng_) - 0.5) * 2.0 * jitter_;
            actual_delay += jitter_offset;
        }
        
        // Ensure delay is not negative
        actual_delay = std::max(0.0, actual_delay);
        
        // Simulate delay by sleeping (in real implementation, this would be handled differently)
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(actual_delay * 1000)));
    }
    
    void apply_jitter(PacketInfo& packet) {
        if (jitter_ <= 0.0) {
            return;
        }
        
        // Jitter is already applied in apply_delay method
        // This method is here for future extensions
    }
};

// Factory function to create NetEm impairment
std::unique_ptr<NetworkImpairment> create_netem_impairment() {
    return std::make_unique<NetEmImpairment>();
}

} // namespace RouterSim
