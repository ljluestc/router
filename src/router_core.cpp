#include "router_sim.h"
#include "frr_integration.h"
#include "traffic_shaping.h"
#include "netem_impairments.h"
#include "cli_interface.h"
#include "yaml_config.h"
#include "packet_processor.h"
#include "routing_table.h"
#include "statistics.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>

using namespace RouterSim;

RouterSimulator::RouterSimulator() 
    : running_(false) {
    // Initialize components
    frr_integration_ = std::make_unique<FRRIntegration>();
    traffic_shaper_ = std::make_unique<TrafficShaper>();
    netem_impairments_ = std::make_unique<NetemImpairments>();
    cli_interface_ = std::make_unique<CLIInterface>(this);
    yaml_config_ = std::make_unique<YamlConfig>();
    packet_processor_ = std::make_unique<PacketProcessor>();
    routing_table_ = std::make_unique<RoutingTable>();
    statistics_ = std::make_unique<Statistics>();
}

RouterSimulator::~RouterSimulator() {
    stop();
}

bool RouterSimulator::initialize(const RouterConfig& config) {
    config_ = config;
    
    // Initialize statistics
    statistics_->register_counter("packets_received", "Total packets received", StatCategory::PACKET_PROCESSING);
    statistics_->register_counter("packets_sent", "Total packets sent", StatCategory::PACKET_PROCESSING);
    statistics_->register_counter("packets_dropped", "Total packets dropped", StatCategory::PACKET_PROCESSING);
    statistics_->register_gauge("active_routes", "Number of active routes", StatCategory::ROUTING);
    statistics_->register_gauge("active_interfaces", "Number of active interfaces", StatCategory::INTERFACE);

    // Initialize packet processor
    if (!packet_processor_->initialize()) {
        std::cerr << "Failed to initialize packet processor\n";
        return false;
    }

    // Set routing table in packet processor
    packet_processor_->set_routing_table(std::shared_ptr<RoutingTable>(routing_table_.get(), [](RoutingTable*){}));

    // Initialize FRR integration
    if (!frr_integration_->initialize(config)) {
        std::cerr << "Failed to initialize FRR integration\n";
        return false;
    }

    // Initialize traffic shaper
    traffic_shaper_->start();

    // Initialize netem impairments
    netem_impairments_->start();

    std::cout << "Router simulator initialized successfully\n";
    return true;
}

bool RouterSimulator::load_config_from_file(const std::string& config_file) {
    return yaml_config_->load_router_config(config_file, config_);
}

bool RouterSimulator::save_config_to_file(const std::string& config_file) {
    return yaml_config_->save_router_config(config_file, config_);
}

bool RouterSimulator::add_interface(const InterfaceConfig& config) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    
    interfaces_[config.name] = config;
    
    // Configure in FRR
    if (!frr_integration_->configure_interface(config.name, config)) {
        std::cerr << "Failed to configure interface " << config.name << " in FRR\n";
        return false;
    }

    // Add to traffic shaper
    ShapingConfig shaping_config;
    shaping_config.rate_bps = config.bandwidth_mbps * 1000000; // Convert to bps
    shaping_config.burst_size = 1000000; // 1MB burst
    traffic_shaper_->add_interface(config.name, shaping_config);

    // Add to netem impairments
    netem_impairments_->add_interface(config.name);

    statistics_->increment_gauge("active_interfaces");
    std::cout << "Added interface " << config.name << " with IP " << config.ip_address << "\n";
    
    return true;
}

bool RouterSimulator::remove_interface(const std::string& name) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    
    auto it = interfaces_.find(name);
    if (it == interfaces_.end()) {
        std::cerr << "Interface " << name << " not found\n";
        return false;
    }

    // Unconfigure from FRR
    frr_integration_->unconfigure_interface(name);

    // Remove from traffic shaper
    traffic_shaper_->remove_interface(name);

    // Remove from netem impairments
    netem_impairments_->remove_interface(name);

    interfaces_.erase(it);
    statistics_->decrement_gauge("active_interfaces");
    std::cout << "Removed interface " << name << "\n";
    
    return true;
}

bool RouterSimulator::update_interface(const InterfaceConfig& config) {
    return add_interface(config); // Same as add for now
}

std::vector<InterfaceConfig> RouterSimulator::get_interfaces() const {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    
    std::vector<InterfaceConfig> result;
    for (const auto& pair : interfaces_) {
        result.push_back(pair.second);
    }
    return result;
}

bool RouterSimulator::start_protocols() {
    bool success = true;

    if (config_.enable_bgp) {
        BGPConfig bgp_config;
        bgp_config.as_number = config_.as_number;
        bgp_config.router_id = config_.router_id;
        bgp_config.networks = {"0.0.0.0/0"}; // Default route
        
        if (!frr_integration_->start_bgp(bgp_config)) {
            std::cerr << "Failed to start BGP\n";
            success = false;
        } else {
            std::cout << "BGP started\n";
        }
    }

    if (config_.enable_ospf) {
        OSPFConfig ospf_config;
        ospf_config.area_id = config_.area_id;
        ospf_config.router_id = config_.router_id;
        ospf_config.networks = {"0.0.0.0/0"}; // Default route
        
        if (!frr_integration_->start_ospf(ospf_config)) {
            std::cerr << "Failed to start OSPF\n";
            success = false;
        } else {
            std::cout << "OSPF started\n";
        }
    }

    if (config_.enable_isis) {
        ISISConfig isis_config;
        isis_config.system_id = config_.system_id;
        isis_config.area_id = config_.area_id;
        isis_config.networks = {"0.0.0.0/0"}; // Default route
        
        if (!frr_integration_->start_isis(isis_config)) {
            std::cerr << "Failed to start IS-IS\n";
            success = false;
        } else {
            std::cout << "IS-IS started\n";
        }
    }

    return success;
}

bool RouterSimulator::stop_protocols() {
    bool success = true;

    if (frr_integration_->is_bgp_running()) {
        if (!frr_integration_->stop_bgp()) {
            std::cerr << "Failed to stop BGP\n";
            success = false;
        } else {
            std::cout << "BGP stopped\n";
        }
    }

    if (frr_integration_->is_ospf_running()) {
        if (!frr_integration_->stop_ospf()) {
            std::cerr << "Failed to stop OSPF\n";
            success = false;
        } else {
            std::cout << "OSPF stopped\n";
        }
    }

    if (frr_integration_->is_isis_running()) {
        if (!frr_integration_->stop_isis()) {
            std::cerr << "Failed to stop IS-IS\n";
            success = false;
        } else {
            std::cout << "IS-IS stopped\n";
        }
    }

    return success;
}

bool RouterSimulator::is_protocol_running(const std::string& protocol) const {
    if (protocol == "bgp") return frr_integration_->is_bgp_running();
    if (protocol == "ospf") return frr_integration_->is_ospf_running();
    if (protocol == "isis") return frr_integration_->is_isis_running();
    return false;
}

bool RouterSimulator::configure_traffic_shaping(const std::string& interface, const ShapingConfig& config) {
    return traffic_shaper_->update_interface_config(interface, config);
}

bool RouterSimulator::enable_wfq(const std::string& interface, uint32_t num_queues) {
    return traffic_shaper_->enable_wfq(interface, num_queues);
}

bool RouterSimulator::configure_impairments(const std::string& interface, const ImpairmentConfig& config) {
    return netem_impairments_->configure_impairments(interface, config);
}

bool RouterSimulator::clear_impairments(const std::string& interface) {
    return netem_impairments_->clear_impairments(interface);
}

bool RouterSimulator::send_packet(const Packet& packet) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    packet_queue_.push(packet);
    statistics_->increment_counter("packets_sent");
    return true;
}

bool RouterSimulator::receive_packet(Packet& packet) {
    std::lock_guard<std::mutex> lock(packet_queue_mutex_);
    
    if (packet_queue_.empty()) {
        return false;
    }
    
    packet = packet_queue_.front();
    packet_queue_.pop();
    statistics_->increment_counter("packets_received");
    return true;
}

void RouterSimulator::process_packets() {
    packet_processing_loop();
}

void RouterSimulator::packet_processing_loop() {
    while (running_) {
        Packet packet;
        if (receive_packet(packet)) {
            // Apply traffic shaping
            traffic_shaper_->shape_packet(packet.source_interface, packet);
            
            // Apply network impairments
            netem_impairments_->process_packet(packet.source_interface, packet);
            
            // Process packet
            auto result = packet_processor_->process_packet(packet);
            
            if (result == ProcessingResult::FORWARD) {
                // Forward packet
                std::string next_hop = packet_processor_->lookup_route(packet.destination_interface);
                if (!next_hop.empty()) {
                    packet.destination_interface = next_hop;
                    send_packet(packet);
                } else {
                    statistics_->increment_counter("packets_dropped");
                }
            } else if (result == ProcessingResult::DROP) {
                statistics_->increment_counter("packets_dropped");
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::map<std::string, uint64_t> RouterSimulator::get_interface_stats(const std::string& interface) const {
    return traffic_shaper_->get_interface_stats(interface);
}

std::map<std::string, uint64_t> RouterSimulator::get_protocol_stats(const std::string& protocol) const {
    if (protocol == "bgp") return frr_integration_->get_bgp_stats();
    if (protocol == "ospf") return frr_integration_->get_ospf_stats();
    if (protocol == "isis") return frr_integration_->get_isis_stats();
    return {};
}

void RouterSimulator::reset_statistics() {
    statistics_->reset_statistics();
    traffic_shaper_->reset_statistics();
    netem_impairments_->reset_statistics();
}

void RouterSimulator::start_cli() {
    cli_interface_->start();
}

void RouterSimulator::stop_cli() {
    cli_interface_->stop();
}

bool RouterSimulator::load_scenario(const std::string& scenario_file) {
    return yaml_config_->load_scenarios(scenario_file);
}

bool RouterSimulator::run_scenario(const std::string& scenario_name) {
    return yaml_config_->execute_scenario(scenario_name);
}

std::vector<std::string> RouterSimulator::list_scenarios() const {
    auto scenarios = yaml_config_->get_scenarios();
    std::vector<std::string> names;
    for (const auto& scenario : scenarios) {
        names.push_back(scenario.name);
    }
    return names;
}

void RouterSimulator::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    
    // Start protocols
    start_protocols();
    
    // Start packet processing thread
    packet_processing_thread_ = std::thread(&RouterSimulator::packet_processing_loop, this);
    
    std::cout << "Router simulator started\n";
}

void RouterSimulator::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Stop packet processing thread
    if (packet_processing_thread_.joinable()) {
        packet_processing_thread_.join();
    }
    
    // Stop protocols
    stop_protocols();
    
    // Stop components
    traffic_shaper_->stop();
    netem_impairments_->stop();
    cli_interface_->stop();
    
    std::cout << "Router simulator stopped\n";
}

bool RouterSimulator::is_running() const {
    return running_;
}
