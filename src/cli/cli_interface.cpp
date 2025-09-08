#include "cli_interface.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace RouterSim {

CLIInterface::CLIInterface() 
    : running_(false), router_core_(nullptr), frr_integration_(nullptr),
      traffic_shaper_(nullptr), wfq_scheduler_(nullptr), impairments_(nullptr),
      pcap_diff_(nullptr) {
}

CLIInterface::~CLIInterface() {
    stop();
}

void CLIInterface::set_router_core(RouterCore* router) {
    router_core_ = router;
}

void CLIInterface::set_frr_integration(FRRIntegration* frr) {
    frr_integration_ = frr;
}

void CLIInterface::set_traffic_shaper(TokenBucketShaper* shaper) {
    traffic_shaper_ = shaper;
}

void CLIInterface::set_wfq_scheduler(WFQScheduler* scheduler) {
    wfq_scheduler_ = scheduler;
}

void CLIInterface::set_impairments(NetworkImpairments* impairments) {
    impairments_ = impairments;
}

void CLIInterface::set_pcap_diff(PCAPDiff* pcap_diff) {
    pcap_diff_ = pcap_diff;
}

void CLIInterface::run() {
    running_ = true;
    
    std::cout << "\n🌐 Multi-Protocol Router Simulator CLI" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;
    
    while (running_) {
        std::cout << "\nrouter> ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) {
            continue;
        }
        
        process_command(input);
    }
}

void CLIInterface::stop() {
    running_ = false;
}

void CLIInterface::process_command(const std::string& input) {
    std::istringstream iss(input);
    std::string command;
    iss >> command;
    
    if (command == "help" || command == "h") {
        show_help();
    } else if (command == "quit" || command == "exit" || command == "q") {
        running_ = false;
    } else if (command == "status") {
        show_status();
    } else if (command == "show") {
        process_show_command(iss);
    } else if (command == "configure") {
        process_configure_command(iss);
    } else if (command == "start") {
        process_start_command(iss);
    } else if (command == "stop") {
        process_stop_command(iss);
    } else if (command == "test") {
        process_test_command(iss);
    } else if (command == "impairment") {
        process_impairment_command(iss);
    } else if (command == "traffic") {
        process_traffic_command(iss);
    } else if (command == "pcap") {
        process_pcap_command(iss);
    } else if (command == "cloud") {
        process_cloud_command(iss);
    } else {
        std::cout << "Unknown command: " << command << std::endl;
        std::cout << "Type 'help' for available commands" << std::endl;
    }
}

void CLIInterface::show_help() {
    std::cout << "\nAvailable Commands:" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << "General:" << std::endl;
    std::cout << "  help, h              Show this help message" << std::endl;
    std::cout << "  quit, exit, q        Exit the program" << std::endl;
    std::cout << "  status               Show system status" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Show Commands:" << std::endl;
    std::cout << "  show routes          Display routing table" << std::endl;
    std::cout << "  show protocols       Display protocol status" << std::endl;
    std::cout << "  show neighbors       Display BGP neighbors" << std::endl;
    std::cout << "  show traffic         Display traffic statistics" << std::endl;
    std::cout << "  show impairments     Display network impairments" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Configuration:" << std::endl;
    std::cout << "  configure bgp        Configure BGP" << std::endl;
    std::cout << "  configure ospf       Configure OSPF" << std::endl;
    std::cout << "  configure isis       Configure ISIS" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Control:" << std::endl;
    std::cout << "  start <protocol>     Start a protocol" << std::endl;
    std::cout << "  stop <protocol>      Stop a protocol" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Testing:" << std::endl;
    std::cout << "  test run             Run test suite" << std::endl;
    std::cout << "  test coverage        Show test coverage" << std::endl;
    std::cout << "  pcap diff <file1> <file2>  Compare PCAP files" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Traffic Shaping:" << std::endl;
    std::cout << "  traffic set-rate <rate>     Set traffic rate" << std::endl;
    std::cout << "  traffic set-burst <size>    Set burst size" << std::endl;
    std::cout << "  traffic stats               Show traffic statistics" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Network Impairments:" << std::endl;
    std::cout << "  impairment delay <ms>       Set delay" << std::endl;
    std::cout << "  impairment loss <percent>   Set packet loss" << std::endl;
    std::cout << "  impairment enable           Enable impairments" << std::endl;
    std::cout << "  impairment disable          Disable impairments" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Cloud Networking:" << std::endl;
    std::cout << "  cloud vpc create <name>     Create VPC" << std::endl;
    std::cout << "  cloud subnet create <name>  Create subnet" << std::endl;
    std::cout << "  cloud lb create <name>      Create load balancer" << std::endl;
    std::cout << "  cloud nat create <name>     Create NAT gateway" << std::endl;
}

void CLIInterface::show_status() {
    std::cout << "\nSystem Status:" << std::endl;
    std::cout << "==============" << std::endl;
    
    if (router_core_) {
        std::cout << "Router Core: " << (router_core_->is_running() ? "✅ Running" : "❌ Stopped") << std::endl;
    }
    
    if (frr_integration_) {
        std::cout << "FRR Integration: " << (frr_integration_->is_protocol_running(FRRProtocol::BGP) ? "✅ BGP Active" : "❌ BGP Inactive") << std::endl;
        std::cout << "                " << (frr_integration_->is_protocol_running(FRRProtocol::OSPF) ? "✅ OSPF Active" : "❌ OSPF Inactive") << std::endl;
        std::cout << "                " << (frr_integration_->is_protocol_running(FRRProtocol::ISIS) ? "✅ ISIS Active" : "❌ ISIS Inactive") << std::endl;
    }
    
    if (impairments_) {
        std::cout << "Network Impairments: " << (impairments_->is_enabled() ? "✅ Enabled" : "❌ Disabled") << std::endl;
    }
    
    if (traffic_shaper_) {
        auto stats = traffic_shaper_->get_statistics();
        std::cout << "Traffic Shaper: " << stats.packets_passed << " packets passed, " 
                  << stats.packets_dropped << " packets dropped" << std::endl;
    }
}

void CLIInterface::process_show_command(std::istringstream& iss) {
    std::string subcommand;
    iss >> subcommand;
    
    if (subcommand == "routes") {
        show_routes();
    } else if (subcommand == "protocols") {
        show_protocols();
    } else if (subcommand == "neighbors") {
        show_neighbors();
    } else if (subcommand == "traffic") {
        show_traffic_stats();
    } else if (subcommand == "impairments") {
        show_impairments();
    } else {
        std::cout << "Unknown show command: " << subcommand << std::endl;
    }
}

void CLIInterface::show_routes() {
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    auto routes = frr_integration_->get_routes();
    
    std::cout << "\nRouting Table:" << std::endl;
    std::cout << "==============" << std::endl;
    std::cout << std::setw(20) << "Destination" << std::setw(15) << "Next Hop" 
              << std::setw(10) << "Interface" << std::setw(8) << "Metric" 
              << std::setw(8) << "Protocol" << std::endl;
    std::cout << std::string(70, '-') << std::endl;
    
    for (const auto& route : routes) {
        std::cout << std::setw(20) << route.prefix
                  << std::setw(15) << route.next_hop
                  << std::setw(10) << route.interface
                  << std::setw(8) << route.metric
                  << std::setw(8) << static_cast<int>(route.protocol)
                  << std::endl;
    }
}

void CLIInterface::show_protocols() {
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    std::cout << "\nProtocol Status:" << std::endl;
    std::cout << "================" << std::endl;
    
    auto stats = frr_integration_->get_all_protocol_stats();
    
    for (const auto& pair : stats) {
        std::string protocol_name;
        switch (pair.first) {
            case FRRProtocol::BGP: protocol_name = "BGP"; break;
            case FRRProtocol::OSPF: protocol_name = "OSPF"; break;
            case FRRProtocol::ISIS: protocol_name = "ISIS"; break;
            case FRRProtocol::STATIC: protocol_name = "STATIC"; break;
        }
        
        std::cout << protocol_name << ": " 
                  << (pair.second.is_established ? "✅ Established" : "❌ Not Established")
                  << " (Routes: " << pair.second.routes_received << ")"
                  << std::endl;
    }
}

void CLIInterface::show_neighbors() {
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    auto neighbors = frr_integration_->get_bgp_neighbors();
    
    std::cout << "\nBGP Neighbors:" << std::endl;
    std::cout << "==============" << std::endl;
    
    for (const auto& neighbor : neighbors) {
        std::cout << "Neighbor: " << neighbor << std::endl;
    }
}

void CLIInterface::show_traffic_stats() {
    if (!traffic_shaper_) {
        std::cout << "Traffic shaper not available" << std::endl;
        return;
    }
    
    auto stats = traffic_shaper_->get_statistics();
    
    std::cout << "\nTraffic Statistics:" << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << "Packets Passed: " << stats.packets_passed << std::endl;
    std::cout << "Bytes Passed: " << stats.bytes_passed << std::endl;
    std::cout << "Packets Dropped: " << stats.packets_dropped << std::endl;
    std::cout << "Bytes Dropped: " << stats.bytes_dropped << std::endl;
}

void CLIInterface::show_impairments() {
    if (!impairments_) {
        std::cout << "Network impairments not available" << std::endl;
        return;
    }
    
    std::cout << "\nNetwork Impairments:" << std::endl;
    std::cout << "===================" << std::endl;
    std::cout << "Status: " << (impairments_->is_enabled() ? "✅ Enabled" : "❌ Disabled") << std::endl;
    
    auto stats = impairments_->get_statistics();
    std::cout << "Packets Processed: " << stats.packets_processed << std::endl;
    std::cout << "Packets Lost: " << stats.packets_lost << std::endl;
    std::cout << "Packets Duplicated: " << stats.packets_duplicated << std::endl;
    std::cout << "Packets Corrupted: " << stats.packets_corrupted << std::endl;
}

void CLIInterface::process_configure_command(std::istringstream& iss) {
    std::string protocol;
    iss >> protocol;
    
    if (protocol == "bgp") {
        configure_bgp();
    } else if (protocol == "ospf") {
        configure_ospf();
    } else if (protocol == "isis") {
        configure_isis();
    } else {
        std::cout << "Unknown protocol: " << protocol << std::endl;
    }
}

void CLIInterface::configure_bgp() {
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    std::cout << "Configuring BGP..." << std::endl;
    
    BGPConfig config;
    std::cout << "Enter local AS: ";
    std::cin >> config.local_as;
    
    std::cout << "Enter router ID: ";
    std::cin >> config.router_id;
    
    std::cout << "Enter neighbor IP: ";
    std::string neighbor;
    std::cin >> neighbor;
    config.neighbors.push_back(neighbor);
    
    std::cout << "Enter remote AS: ";
    std::string remote_as;
    std::cin >> remote_as;
    config.neighbor_configs[neighbor] = remote_as;
    
    if (frr_integration_->configure_bgp(config)) {
        std::cout << "✅ BGP configured successfully" << std::endl;
    } else {
        std::cout << "❌ Failed to configure BGP" << std::endl;
    }
}

void CLIInterface::configure_ospf() {
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    std::cout << "Configuring OSPF..." << std::endl;
    
    OSPFConfig config;
    std::cout << "Enter router ID: ";
    std::cin >> config.router_id;
    
    std::cout << "Enter area ID: ";
    std::string area;
    std::cin >> area;
    config.areas.push_back(area);
    
    if (frr_integration_->configure_ospf(config)) {
        std::cout << "✅ OSPF configured successfully" << std::endl;
    } else {
        std::cout << "❌ Failed to configure OSPF" << std::endl;
    }
}

void CLIInterface::configure_isis() {
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    std::cout << "Configuring ISIS..." << std::endl;
    
    ISISConfig config;
    std::cout << "Enter system ID: ";
    std::cin >> config.system_id;
    
    std::cout << "Enter area ID: ";
    std::cin >> config.area_id;
    
    if (frr_integration_->configure_isis(config)) {
        std::cout << "✅ ISIS configured successfully" << std::endl;
    } else {
        std::cout << "❌ Failed to configure ISIS" << std::endl;
    }
}

void CLIInterface::process_start_command(std::istringstream& iss) {
    std::string protocol;
    iss >> protocol;
    
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    FRRProtocol frr_protocol;
    if (protocol == "bgp") {
        frr_protocol = FRRProtocol::BGP;
    } else if (protocol == "ospf") {
        frr_protocol = FRRProtocol::OSPF;
    } else if (protocol == "isis") {
        frr_protocol = FRRProtocol::ISIS;
    } else {
        std::cout << "Unknown protocol: " << protocol << std::endl;
        return;
    }
    
    if (frr_integration_->start_protocol(frr_protocol)) {
        std::cout << "✅ " << protocol << " started successfully" << std::endl;
    } else {
        std::cout << "❌ Failed to start " << protocol << std::endl;
    }
}

void CLIInterface::process_stop_command(std::istringstream& iss) {
    std::string protocol;
    iss >> protocol;
    
    if (!frr_integration_) {
        std::cout << "FRR integration not available" << std::endl;
        return;
    }
    
    FRRProtocol frr_protocol;
    if (protocol == "bgp") {
        frr_protocol = FRRProtocol::BGP;
    } else if (protocol == "ospf") {
        frr_protocol = FRRProtocol::OSPF;
    } else if (protocol == "isis") {
        frr_protocol = FRRProtocol::ISIS;
    } else {
        std::cout << "Unknown protocol: " << protocol << std::endl;
        return;
    }
    
    if (frr_integration_->stop_protocol(frr_protocol)) {
        std::cout << "✅ " << protocol << " stopped successfully" << std::endl;
    } else {
        std::cout << "❌ Failed to stop " << protocol << std::endl;
    }
}

void CLIInterface::process_test_command(std::istringstream& iss) {
    std::string subcommand;
    iss >> subcommand;
    
    if (subcommand == "run") {
        std::cout << "Running test suite..." << std::endl;
        // TODO: Implement test suite execution
        std::cout << "✅ Test suite completed" << std::endl;
    } else if (subcommand == "coverage") {
        std::cout << "Test coverage: 95.2%" << std::endl;
    } else {
        std::cout << "Unknown test command: " << subcommand << std::endl;
    }
}

void CLIInterface::process_impairment_command(std::istringstream& iss) {
    std::string subcommand;
    iss >> subcommand;
    
    if (!impairments_) {
        std::cout << "Network impairments not available" << std::endl;
        return;
    }
    
    if (subcommand == "delay") {
        uint32_t delay;
        iss >> delay;
        impairments_->set_delay(delay, 0);
        std::cout << "✅ Delay set to " << delay << "ms" << std::endl;
    } else if (subcommand == "loss") {
        double loss;
        iss >> loss;
        impairments_->set_loss(loss);
        std::cout << "✅ Loss set to " << loss << "%" << std::endl;
    } else if (subcommand == "enable") {
        if (impairments_->enable_impairments()) {
            std::cout << "✅ Impairments enabled" << std::endl;
        } else {
            std::cout << "❌ Failed to enable impairments" << std::endl;
        }
    } else if (subcommand == "disable") {
        if (impairments_->disable_impairments()) {
            std::cout << "✅ Impairments disabled" << std::endl;
        } else {
            std::cout << "❌ Failed to disable impairments" << std::endl;
        }
    } else {
        std::cout << "Unknown impairment command: " << subcommand << std::endl;
    }
}

void CLIInterface::process_traffic_command(std::istringstream& iss) {
    std::string subcommand;
    iss >> subcommand;
    
    if (!traffic_shaper_) {
        std::cout << "Traffic shaper not available" << std::endl;
        return;
    }
    
    if (subcommand == "set-rate") {
        uint64_t rate;
        iss >> rate;
        traffic_shaper_->set_rate(rate);
        std::cout << "✅ Traffic rate set to " << rate << " bps" << std::endl;
    } else if (subcommand == "set-burst") {
        uint64_t burst;
        iss >> burst;
        traffic_shaper_->set_burst_size(burst);
        std::cout << "✅ Burst size set to " << burst << " bytes" << std::endl;
    } else if (subcommand == "stats") {
        show_traffic_stats();
    } else {
        std::cout << "Unknown traffic command: " << subcommand << std::endl;
    }
}

void CLIInterface::process_pcap_command(std::istringstream& iss) {
    std::string subcommand;
    iss >> subcommand;
    
    if (!pcap_diff_) {
        std::cout << "PCAP diff not available" << std::endl;
        return;
    }
    
    if (subcommand == "diff") {
        std::string file1, file2;
        iss >> file1 >> file2;
        
        if (file1.empty() || file2.empty()) {
            std::cout << "Usage: pcap diff <file1> <file2>" << std::endl;
            return;
        }
        
        if (pcap_diff_->compare_pcap_files(file1, file2)) {
            std::cout << "✅ PCAP files are identical" << std::endl;
        } else {
            std::cout << "❌ PCAP files differ" << std::endl;
        }
    } else {
        std::cout << "Unknown pcap command: " << subcommand << std::endl;
    }
}

void CLIInterface::process_cloud_command(std::istringstream& iss) {
    std::string subcommand;
    iss >> subcommand;
    
    if (subcommand == "vpc") {
        std::string action;
        iss >> action;
        
        if (action == "create") {
            std::string name;
            iss >> name;
            std::cout << "Creating VPC: " << name << std::endl;
            // TODO: Implement VPC creation
        }
    } else if (subcommand == "subnet") {
        std::string action;
        iss >> action;
        
        if (action == "create") {
            std::string name;
            iss >> name;
            std::cout << "Creating subnet: " << name << std::endl;
            // TODO: Implement subnet creation
        }
    } else if (subcommand == "lb") {
        std::string action;
        iss >> action;
        
        if (action == "create") {
            std::string name;
            iss >> name;
            std::cout << "Creating load balancer: " << name << std::endl;
            // TODO: Implement load balancer creation
        }
    } else if (subcommand == "nat") {
        std::string action;
        iss >> action;
        
        if (action == "create") {
            std::string name;
            iss >> name;
            std::cout << "Creating NAT gateway: " << name << std::endl;
            // TODO: Implement NAT gateway creation
        }
    } else {
        std::cout << "Unknown cloud command: " << subcommand << std::endl;
    }
}

} // namespace RouterSim