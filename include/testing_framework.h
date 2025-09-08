#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <functional>
#include <gtest/gtest.h>
#include <pcap.h>

namespace router_sim::testing {

// Test Configuration
struct TestConfig {
    std::string test_name;
    std::string description;
    std::map<std::string, std::string> parameters;
    std::chrono::milliseconds timeout;
    bool enable_pcap_capture;
    bool enable_coverage;
    std::string output_directory;
    
    TestConfig() : timeout(std::chrono::seconds(30)), enable_pcap_capture(true), 
                  enable_coverage(false), output_directory("./test_output") {}
};

// Packet Capture Information
struct PacketInfo {
    std::vector<uint8_t> data;
    size_t length;
    std::chrono::steady_clock::time_point timestamp;
    std::string interface;
    uint32_t packet_number;
    
    PacketInfo() : length(0), packet_number(0) {
        timestamp = std::chrono::steady_clock::now();
    }
};

// Test Statistics
struct TestStatistics {
    uint64_t packets_captured;
    uint64_t packets_expected;
    uint64_t packets_matched;
    uint64_t packets_dropped;
    uint64_t bytes_captured;
    uint64_t bytes_expected;
    double packet_loss_percentage;
    double throughput_mbps;
    std::chrono::milliseconds test_duration;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    
    void reset() {
        packets_captured = 0;
        packets_expected = 0;
        packets_matched = 0;
        packets_dropped = 0;
        bytes_captured = 0;
        bytes_expected = 0;
        packet_loss_percentage = 0.0;
        throughput_mbps = 0.0;
        test_duration = std::chrono::milliseconds(0);
        start_time = std::chrono::steady_clock::now();
        end_time = start_time;
    }
};

// PCAP Diff Result
struct PcapDiffResult {
    bool packets_match;
    uint64_t total_packets;
    uint64_t matching_packets;
    uint64_t different_packets;
    uint64_t missing_packets;
    uint64_t extra_packets;
    std::vector<std::string> differences;
    double similarity_percentage;
    
    PcapDiffResult() : packets_match(false), total_packets(0), matching_packets(0),
                      different_packets(0), missing_packets(0), extra_packets(0),
                      similarity_percentage(0.0) {}
};

// Base Test Case
class RouterTestCase : public ::testing::Test {
public:
    virtual ~RouterTestCase() = default;
    
    // Setup and teardown
    virtual void SetUp() override;
    virtual void TearDown() override;
    
    // Test execution
    virtual bool run_test() = 0;
    virtual bool validate_results() = 0;
    
    // Configuration
    void set_test_config(const TestConfig& config);
    TestConfig get_test_config() const;
    
    // Statistics
    TestStatistics get_statistics() const;
    void reset_statistics();
    
    // PCAP operations
    bool start_pcap_capture(const std::string& interface);
    bool stop_pcap_capture();
    std::vector<PacketInfo> get_captured_packets() const;
    
    // Test utilities
    bool wait_for_condition(std::function<bool()> condition, std::chrono::milliseconds timeout);
    bool send_test_packet(const std::vector<uint8_t>& data, const std::string& interface);
    bool expect_packet(const std::vector<uint8_t>& expected_data, std::chrono::milliseconds timeout);

protected:
    TestConfig config_;
    TestStatistics statistics_;
    std::vector<PacketInfo> captured_packets_;
    pcap_t* pcap_handle_;
    bool pcap_capturing_;
    std::string pcap_interface_;
};

// PCAP Diff Engine
class PcapDiffEngine {
public:
    PcapDiffEngine();
    virtual ~PcapDiffEngine();
    
    // Core operations
    bool load_pcap_file(const std::string& filename);
    bool save_pcap_file(const std::string& filename, const std::vector<PacketInfo>& packets);
    
    // Comparison operations
    PcapDiffResult compare_pcaps(const std::string& file1, const std::string& file2);
    PcapDiffResult compare_packets(const std::vector<PacketInfo>& packets1, 
                                  const std::vector<PacketInfo>& packets2);
    
    // Analysis operations
    std::vector<std::string> analyze_differences(const PcapDiffResult& result);
    bool generate_diff_report(const PcapDiffResult& result, const std::string& output_file);
    
    // Filtering operations
    std::vector<PacketInfo> filter_packets(const std::vector<PacketInfo>& packets,
                                          std::function<bool(const PacketInfo&)> filter);
    std::vector<PacketInfo> filter_by_protocol(const std::vector<PacketInfo>& packets,
                                              const std::string& protocol);
    std::vector<PacketInfo> filter_by_time_range(const std::vector<PacketInfo>& packets,
                                                std::chrono::steady_clock::time_point start,
                                                std::chrono::steady_clock::time_point end);

private:
    bool parse_pcap_file(const std::string& filename, std::vector<PacketInfo>& packets);
    bool write_pcap_file(const std::string& filename, const std::vector<PacketInfo>& packets);
    bool packets_equal(const PacketInfo& p1, const PacketInfo& p2) const;
    double calculate_similarity(const std::vector<PacketInfo>& packets1,
                               const std::vector<PacketInfo>& packets2) const;
};

// Coverage Collector
class CoverageCollector {
public:
    CoverageCollector();
    virtual ~CoverageCollector();
    
    // Coverage operations
    bool start_coverage_collection();
    bool stop_coverage_collection();
    bool is_collecting() const;
    
    // Coverage data
    std::map<std::string, double> get_line_coverage() const;
    std::map<std::string, double> get_function_coverage() const;
    std::map<std::string, double> get_branch_coverage() const;
    double get_overall_coverage() const;
    
    // Coverage reporting
    bool generate_coverage_report(const std::string& output_file);
    bool generate_html_report(const std::string& output_file);
    bool generate_lcov_report(const std::string& output_file);

private:
    bool collecting_;
    std::map<std::string, double> line_coverage_;
    std::map<std::string, double> function_coverage_;
    std::map<std::string, double> branch_coverage_;
    
    void collect_coverage_data();
    bool parse_gcov_data();
    bool generate_lcov_data(const std::string& output_file);
};

// Test Suite Manager
class TestSuiteManager {
public:
    TestSuiteManager();
    virtual ~TestSuiteManager();
    
    // Test management
    bool add_test_case(std::shared_ptr<RouterTestCase> test_case);
    bool remove_test_case(const std::string& test_name);
    std::vector<std::string> get_test_names() const;
    
    // Test execution
    bool run_all_tests();
    bool run_test(const std::string& test_name);
    bool run_tests(const std::vector<std::string>& test_names);
    
    // Test configuration
    bool set_global_config(const TestConfig& config);
    TestConfig get_global_config() const;
    
    // Coverage and reporting
    bool enable_coverage(bool enable);
    bool generate_test_report(const std::string& output_file);
    bool generate_coverage_report(const std::string& output_file);
    
    // Statistics
    std::map<std::string, TestStatistics> get_all_statistics() const;
    TestStatistics get_test_statistics(const std::string& test_name) const;

private:
    std::map<std::string, std::shared_ptr<RouterTestCase>> test_cases_;
    TestConfig global_config_;
    std::unique_ptr<CoverageCollector> coverage_collector_;
    std::map<std::string, TestStatistics> test_statistics_;
    
    bool run_single_test(std::shared_ptr<RouterTestCase> test_case);
    void collect_test_statistics(const std::string& test_name, const TestStatistics& stats);
};

// Specific Test Cases
class BGPConvergenceTest : public RouterTestCase {
public:
    void SetUp() override;
    bool run_test() override;
    bool validate_results() override;
    
private:
    bool test_bgp_route_advertisement();
    bool test_bgp_route_withdrawal();
    bool test_bgp_convergence_time();
};

class OSPFConvergenceTest : public RouterTestCase {
public:
    void SetUp() override;
    bool run_test() override;
    bool validate_results() override;
    
private:
    bool test_ospf_hello_exchange();
    bool test_ospf_lsa_flooding();
    bool test_ospf_convergence_time();
};

class TrafficShapingTest : public RouterTestCase {
public:
    void SetUp() override;
    bool run_test() override;
    bool validate_results() override;
    
private:
    bool test_token_bucket_shaping();
    bool test_wfq_scheduling();
    bool test_bandwidth_limitation();
};

class NetworkImpairmentTest : public RouterTestCase {
public:
    void SetUp() override;
    bool run_test() override;
    bool validate_results() override;
    
private:
    bool test_delay_impairment();
    bool test_packet_loss_impairment();
    bool test_jitter_impairment();
};

// Test Fixtures
class RouterTestFixture : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    std::unique_ptr<TestSuiteManager> test_manager_;
    std::unique_ptr<PcapDiffEngine> pcap_diff_engine_;
    std::unique_ptr<CoverageCollector> coverage_collector_;
};

// Test Macros
#define ROUTER_TEST(test_name) \
    TEST_F(RouterTestFixture, test_name)

#define ROUTER_TEST_WITH_PCAP(test_name) \
    TEST_F(RouterTestFixture, test_name) { \
        config_.enable_pcap_capture = true; \
        set_test_config(config_); \
    }

#define ROUTER_TEST_WITH_COVERAGE(test_name) \
    TEST_F(RouterTestFixture, test_name) { \
        config_.enable_coverage = true; \
        set_test_config(config_); \
    }

#define EXPECT_PACKET_MATCH(expected, actual) \
    EXPECT_TRUE(pcap_diff_engine_->compare_packets(expected, actual).packets_match)

#define EXPECT_PACKET_LOSS(expected_packets, actual_packets, max_loss_percentage) \
    { \
        auto result = pcap_diff_engine_->compare_packets(expected_packets, actual_packets); \
        double loss_percentage = (1.0 - result.similarity_percentage / 100.0) * 100.0; \
        EXPECT_LE(loss_percentage, max_loss_percentage); \
    }

} // namespace router_sim::testing
