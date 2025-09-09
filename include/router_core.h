#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>

namespace RouterSim {

// Forward declarations
class ProtocolHandler;
class TrafficShaper;
class NetworkImpairment;
class FRRIntegration;
class ClickHouseClient;
class CLIInterface;
class ConfigManager;

// Router core class
class RouterCore {
public:
    RouterCore();
    ~RouterCore();

    // Core lifecycle
    bool initialize(const std::string& config_file);
    void start();
    void stop();
    bool is_running() const;

    // Protocol management
    void add_protocol(const std::string& name, std::shared_ptr<ProtocolHandler> handler);
    void remove_protocol(const std::string& name);
    std::shared_ptr<ProtocolHandler> get_protocol(const std::string& name);

    // Traffic shaping
    void set_traffic_shaper(std::shared_ptr<TrafficShaper> shaper);
    std::shared_ptr<TrafficShaper> get_traffic_shaper() const;

    // Network impairments
    void add_impairment(const std::string& interface, std::shared_ptr<NetworkImpairment> impairment);
    void remove_impairment(const std::string& interface);

    // Configuration
    void load_config(const std::string& config_file);
    void save_config(const std::string& config_file);

    // Statistics and monitoring
    std::map<std::string, uint64_t> get_statistics() const;
    void reset_statistics();

    // Event callbacks
    void set_packet_callback(std::function<void(const std::string&, const std::vector<uint8_t>&)> callback);
    void set_route_callback(std::function<void(const std::string&, const std::string&)> callback);

private:
    void packet_processing_loop();
    void protocol_processing_loop();
    void monitoring_loop();

    std::atomic<bool> running_;
    std::thread packet_thread_;
    std::thread protocol_thread_;
    std::thread monitoring_thread_;

    std::map<std::string, std::shared_ptr<ProtocolHandler>> protocols_;
    std::shared_ptr<TrafficShaper> traffic_shaper_;
    std::map<std::string, std::shared_ptr<NetworkImpairment>> impairments_;
    std::shared_ptr<FRRIntegration> frr_integration_;
    std::shared_ptr<ClickHouseClient> analytics_client_;
    std::shared_ptr<CLIInterface> cli_interface_;
    std::shared_ptr<ConfigManager> config_manager_;

    mutable std::mutex protocols_mutex_;
    mutable std::mutex impairments_mutex_;
    mutable std::mutex statistics_mutex_;

    std::map<std::string, uint64_t> statistics_;
    std::function<void(const std::string&, const std::vector<uint8_t>&)> packet_callback_;
    std::function<void(const std::string&, const std::string&)> route_callback_;
};

// Protocol handler interface
class ProtocolHandler {
public:
    virtual ~ProtocolHandler() = default;
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
    virtual void process_packet(const std::vector<uint8_t>& packet) = 0;
    virtual std::vector<std::string> get_routes() const = 0;
    virtual std::string get_name() const = 0;
    virtual std::map<std::string, uint64_t> get_statistics() const = 0;
};

// BGP Protocol Handler
class BGPHandler : public ProtocolHandler {
public:
    BGPHandler();
    ~BGPHandler() override;

    bool initialize() override;
    void start() override;
    void stop() override;
    bool is_running() const override;
    void process_packet(const std::vector<uint8_t>& packet) override;
    std::vector<std::string> get_routes() const override;
    std::string get_name() const override;
    std::map<std::string, uint64_t> get_statistics() const override;

    // BGP-specific methods
    void add_neighbor(const std::string& neighbor_ip, uint16_t as_number);
    void remove_neighbor(const std::string& neighbor_ip);
    void advertise_route(const std::string& prefix, const std::string& next_hop);
    void withdraw_route(const std::string& prefix);

private:
    std::atomic<bool> running_;
    std::map<std::string, uint16_t> neighbors_;
    std::vector<std::string> advertised_routes_;
    mutable std::mutex neighbors_mutex_;
    mutable std::mutex routes_mutex_;
    std::map<std::string, uint64_t> statistics_;
};

// OSPF Protocol Handler
class OSPFHandler : public ProtocolHandler {
public:
    OSPFHandler();
    ~OSPFHandler() override;

    bool initialize() override;
    void start() override;
    void stop() override;
    bool is_running() const override;
    void process_packet(const std::vector<uint8_t>& packet) override;
    std::vector<std::string> get_routes() const override;
    std::string get_name() const override;
    std::map<std::string, uint64_t> get_statistics() const override;

    // OSPF-specific methods
    void add_interface(const std::string& interface, uint32_t area_id);
    void remove_interface(const std::string& interface);
    void set_router_id(const std::string& router_id);

private:
    std::atomic<bool> running_;
    std::string router_id_;
    std::map<std::string, uint32_t> interfaces_;
    std::vector<std::string> routes_;
    mutable std::mutex interfaces_mutex_;
    mutable std::mutex routes_mutex_;
    std::map<std::string, uint64_t> statistics_;
};

// ISIS Protocol Handler
class ISISHandler : public ProtocolHandler {
public:
    ISISHandler();
    ~ISISHandler() override;

    bool initialize() override;
    void start() override;
    void stop() override;
    bool is_running() const override;
    void process_packet(const std::vector<uint8_t>& packet) override;
    std::vector<std::string> get_routes() const override;
    std::string get_name() const override;
    std::map<std::string, uint64_t> get_statistics() const override;

    // ISIS-specific methods
    void set_system_id(const std::string& system_id);
    void add_interface(const std::string& interface, uint8_t level);
    void remove_interface(const std::string& interface);

private:
    std::atomic<bool> running_;
    std::string system_id_;
    std::map<std::string, uint8_t> interfaces_;
    std::vector<std::string> routes_;
    mutable std::mutex interfaces_mutex_;
    mutable std::mutex routes_mutex_;
    std::map<std::string, uint64_t> statistics_;
};

} // namespace RouterSim
