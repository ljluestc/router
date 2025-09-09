#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace RouterSim {

// Forward declarations
class FRRClient;

// BGP Configuration
struct BGPNeighbor {
    std::string address;
    uint32_t remote_as;
    std::string source_interface;
    std::string password;
};

struct BGPConfig {
    uint32_t as_number;
    std::vector<BGPNeighbor> neighbors;
    std::string router_id;
};

// OSPF Configuration
struct OSPFNetwork {
    std::string address;
    std::string area;
};

struct OSPFConfig {
    std::string router_id;
    std::vector<OSPFNetwork> networks;
    uint32_t hello_interval;
    uint32_t dead_interval;
};

// ISIS Configuration
enum class ISISLevel {
    L1,
    L2,
    L1_L2
};

struct ISISInterface {
    std::string name;
    uint32_t hello_interval;
    uint32_t hello_multiplier;
};

struct ISISConfig {
    std::string tag;
    std::string net_id;
    ISISLevel is_type;
    std::vector<ISISInterface> interfaces;
};

// Route and Interface structures
struct Route {
    std::string destination;
    std::string next_hop;
    std::string protocol;
    uint32_t metric;
    std::string interface;
};

struct Interface {
    std::string name;
    std::string ip_address;
    std::string status;
    uint64_t bytes_in;
    uint64_t bytes_out;
    uint64_t packets_in;
    uint64_t packets_out;
};

// FRR Client class
class FRRClient {
public:
    FRRClient();
    ~FRRClient();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const { return connected_; }

    // Protocol configuration
    bool configureBGP(const BGPConfig& config);
    bool configureOSPF(const OSPFConfig& config);
    bool configureISIS(const ISISConfig& config);

    // Information retrieval
    std::vector<Route> getRoutes();
    std::vector<Interface> getInterfaces();
    std::string getProtocolStatus(const std::string& protocol);

    // Route management
    bool addRoute(const Route& route);
    bool removeRoute(const Route& route);
    bool updateRoute(const Route& route);

    // Interface management
    bool configureInterface(const Interface& interface);
    bool bringUpInterface(const std::string& interface_name);
    bool bringDownInterface(const std::string& interface_name);

private:
    bool connected_;
    int socket_fd_;
    
    // Internal methods
    std::string sendCommand(const std::string& command);
    std::vector<Route> parseRoutes(const std::string& output);
    std::vector<Interface> parseInterfaces(const std::string& output);
    std::string parseProtocolStatus(const std::string& output, const std::string& protocol);
};

// FRR Manager for high-level operations
class FRRManager {
public:
    FRRManager();
    ~FRRManager();

    // Initialization
    bool initialize();
    void shutdown();

    // Protocol management
    bool startProtocol(const std::string& protocol, const std::string& config);
    bool stopProtocol(const std::string& protocol);
    bool restartProtocol(const std::string& protocol);

    // Configuration management
    bool loadConfiguration(const std::string& config_file);
    bool saveConfiguration(const std::string& config_file);
    bool validateConfiguration(const std::string& config_file);

    // Monitoring
    std::map<std::string, std::string> getProtocolStatus();
    std::vector<Route> getAllRoutes();
    std::vector<Interface> getAllInterfaces();
    
    // Event handling
    void registerEventHandler(const std::string& event, std::function<void(const std::string&)> handler);
    void unregisterEventHandler(const std::string& event);

private:
    std::unique_ptr<FRRClient> client_;
    std::map<std::string, std::function<void(const std::string&)>> event_handlers_;
    bool initialized_;
    
    // Internal methods
    bool startFRRDaemon();
    bool stopFRRDaemon();
    void handleFRREvent(const std::string& event);
};

} // namespace RouterSim