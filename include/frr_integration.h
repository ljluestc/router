#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace RouterSim {

// FRR daemon types
enum class FRRDaemon {
    BGP,
    OSPF,
    ISIS,
    ZEBRA,
    STATIC
};

// FRR connection status
enum class FRRStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};

// FRR configuration
struct FRRConfig {
    std::string hostname;
    uint16_t port;
    std::string password;
    bool use_vtysh;
    std::string config_file;
    std::vector<std::string> daemons;
    
    FRRConfig() : port(2605), use_vtysh(true) {}
};

// FRR command result
struct FRRCommandResult {
    bool success;
    std::string output;
    std::string error;
    int exit_code;
    
    FRRCommandResult() : success(false), exit_code(-1) {}
};

// FRR event types
enum class FRREventType {
    ROUTE_ADDED,
    ROUTE_REMOVED,
    ROUTE_UPDATED,
    NEIGHBOR_UP,
    NEIGHBOR_DOWN,
    INTERFACE_UP,
    INTERFACE_DOWN,
    DAEMON_STARTED,
    DAEMON_STOPPED,
    ERROR
};

// FRR event data
struct FRREvent {
    FRREventType type;
    std::string daemon;
    std::string data;
    std::chrono::steady_clock::time_point timestamp;
    
    FRREvent() : type(FRREventType::ERROR) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// FRR event callback
using FRREventCallback = std::function<void(const FRREvent&)>;

// FRR integration class
class FRRIntegration {
public:
    FRRIntegration();
    ~FRRIntegration();

    // Lifecycle
    bool initialize(const FRRConfig& config);
    void start();
    void stop();
    bool is_connected() const;
    FRRStatus get_status() const;

    // Configuration
    void set_config(const FRRConfig& config);
    FRRConfig get_config() const;

    // Command execution
    FRRCommandResult execute_command(const std::string& command);
    FRRCommandResult execute_vtysh_command(const std::string& command);
    FRRCommandResult execute_daemon_command(FRRDaemon daemon, const std::string& command);

    // Daemon management
    bool start_daemon(FRRDaemon daemon);
    bool stop_daemon(FRRDaemon daemon);
    bool restart_daemon(FRRDaemon daemon);
    bool is_daemon_running(FRRDaemon daemon) const;

    // Configuration management
    bool load_config_file(const std::string& filename);
    bool save_config_file(const std::string& filename);
    bool apply_config();
    bool reload_config();

    // Event handling
    void set_event_callback(FRREventCallback callback);
    void remove_event_callback();

    // Statistics
    std::map<std::string, uint64_t> get_statistics() const;
    void reset_statistics();

    // BGP specific methods
    bool add_bgp_neighbor(const std::string& neighbor_ip, uint16_t as_number);
    bool remove_bgp_neighbor(const std::string& neighbor_ip);
    bool advertise_bgp_route(const std::string& prefix, const std::string& next_hop);
    bool withdraw_bgp_route(const std::string& prefix);
    std::vector<std::string> get_bgp_routes() const;
    std::vector<std::string> get_bgp_neighbors() const;

    // OSPF specific methods
    bool add_ospf_interface(const std::string& interface, uint32_t area_id);
    bool remove_ospf_interface(const std::string& interface);
    bool set_ospf_router_id(const std::string& router_id);
    std::vector<std::string> get_ospf_routes() const;
    std::vector<std::string> get_ospf_interfaces() const;

    // ISIS specific methods
    bool set_isis_system_id(const std::string& system_id);
    bool add_isis_interface(const std::string& interface, uint8_t level);
    bool remove_isis_interface(const std::string& interface);
    std::vector<std::string> get_isis_routes() const;
    std::vector<std::string> get_isis_interfaces() const;

    // Zebra specific methods
    bool add_interface(const std::string& interface, const std::string& ip_address, const std::string& netmask);
    bool remove_interface(const std::string& interface);
    bool set_interface_up(const std::string& interface);
    bool set_interface_down(const std::string& interface);
    std::vector<std::string> get_interfaces() const;

private:
    void event_processing_loop();
    void process_frr_output(const std::string& output);
    FRREvent parse_frr_event(const std::string& line);
    std::string build_vtysh_command(const std::string& command);
    std::string build_daemon_command(FRRDaemon daemon, const std::string& command);

    std::atomic<bool> running_;
    std::atomic<FRRStatus> status_;
    FRRConfig config_;
    
    std::thread event_thread_;
    std::unique_ptr<class FRRClient> client_;
    
    mutable std::mutex config_mutex_;
    mutable std::mutex statistics_mutex_;
    
    std::map<std::string, uint64_t> statistics_;
    FRREventCallback event_callback_;
    
    // Daemon status tracking
    std::map<FRRDaemon, bool> daemon_status_;
    mutable std::mutex daemon_mutex_;
};

// FRR client interface
class FRRClient {
public:
    virtual ~FRRClient() = default;
    virtual bool connect(const std::string& hostname, uint16_t port) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    virtual FRRCommandResult execute_command(const std::string& command) = 0;
    virtual void set_output_callback(std::function<void(const std::string&)> callback) = 0;
};

// VTYSH client implementation
class VTYSHClient : public FRRClient {
public:
    VTYSHClient();
    ~VTYSHClient() override;

    bool connect(const std::string& hostname, uint16_t port) override;
    void disconnect() override;
    bool is_connected() const override;
    FRRCommandResult execute_command(const std::string& command) override;
    void set_output_callback(std::function<void(const std::string&)> callback) override;

private:
    bool execute_vtysh(const std::string& command, std::string& output, std::string& error);
    
    std::atomic<bool> connected_;
    std::function<void(const std::string&)> output_callback_;
    mutable std::mutex client_mutex_;
};

// Socket client implementation
class SocketClient : public FRRClient {
public:
    SocketClient();
    ~SocketClient() override;

    bool connect(const std::string& hostname, uint16_t port) override;
    void disconnect() override;
    bool is_connected() const override;
    FRRCommandResult execute_command(const std::string& command) override;
    void set_output_callback(std::function<void(const std::string&)> callback) override;

private:
    bool send_command(const std::string& command);
    bool receive_response(std::string& response);
    
    int socket_fd_;
    std::atomic<bool> connected_;
    std::function<void(const std::string&)> output_callback_;
    mutable std::mutex socket_mutex_;
};

} // namespace RouterSim