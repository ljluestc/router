#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <pcap.h>

namespace router_sim {
namespace testing {

// Packet information structure
struct PacketInfo {
    uint64_t timestamp;
    uint32_t length;
    uint32_t captured_length;
    uint32_t src_ip;
    uint32_t dst_ip;
    uint8_t protocol;
    uint8_t ttl;
    uint8_t tos;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t flags;
    
    PacketInfo() : timestamp(0), length(0), captured_length(0), src_ip(0), dst_ip(0),
                   protocol(0), ttl(0), tos(0), src_port(0), dst_port(0), flags(0) {}
};

// PCAP Diff Testing
class PCAPDiffTester {
public:
    PCAPDiffTester();
    ~PCAPDiffTester();
    
    bool initialize(const std::string& interface);
    bool capture_packets(const std::string& output_file, uint32_t count, uint32_t timeout_ms = 5000);
    
    struct DiffResult {
        bool files_match;
        uint32_t total_packets_1;
        uint32_t total_packets_2;
        uint32_t differences;
        std::string error_message;
        
        DiffResult() : files_match(false), total_packets_1(0), total_packets_2(0), 
                      differences(0) {}
    };
    
    DiffResult compare_pcaps(const std::string& file1, const std::string& file2);
    std::vector<PacketInfo> analyze_pcap(const std::string& filename);

private:
    pcap_t* handle_;
    bool initialized_;
};

// Coverage Testing
class CoverageTester {
public:
    CoverageTester();
    ~CoverageTester();
    
    bool enable();
    bool disable();
    bool is_enabled() const { return enabled_; }
    
    struct CoverageReport {
        uint32_t total_lines;
        uint32_t covered_lines;
        double coverage_percentage;
        std::string error_message;
        
        CoverageReport() : total_lines(0), covered_lines(0), coverage_percentage(0.0) {}
    };
    
    CoverageReport generate_report();

private:
    bool enabled_;
};

// Performance Testing
class PerformanceTester {
public:
    PerformanceTester();
    ~PerformanceTester();
    
    struct PerformanceResult {
        uint32_t iterations;
        uint32_t warmup_iterations;
        uint64_t total_time_ns;
        uint64_t average_time_ns;
        double operations_per_second;
        
        PerformanceResult() : iterations(0), warmup_iterations(0), total_time_ns(0),
                             average_time_ns(0), operations_per_second(0.0) {}
    };
    
    PerformanceResult run_benchmark(std::function<void()> test_function,
                                   uint32_t iterations = 1000,
                                   uint32_t warmup_iterations = 100);

private:
};

// Load Testing
class LoadTester {
public:
    LoadTester();
    ~LoadTester();
    
    bool start(uint32_t num_threads = 4, uint32_t requests_per_second = 100);
    bool stop();
    bool is_running() const { return running_; }
    
    struct LoadTestStats {
        uint64_t total_requests;
        uint64_t total_time_us;
        uint64_t max_time_us;
        uint64_t min_time_us;
        uint64_t average_time_us;
        double requests_per_second;
        
        LoadTestStats() : total_requests(0), total_time_us(0), max_time_us(0),
                         min_time_us(0), average_time_us(0), requests_per_second(0.0) {}
    };
    
    LoadTestStats get_stats() const;

private:
    void worker_thread(uint32_t thread_id);
    
    std::vector<std::thread> threads_;
    std::atomic<bool> running_;
    uint32_t requests_per_second_;
    
    mutable std::mutex stats_mutex_;
    uint64_t total_requests_;
    uint64_t total_time_us_;
    uint64_t max_time_us_;
    uint64_t min_time_us_;
};

// Integration Testing
class IntegrationTester {
public:
    IntegrationTester();
    ~IntegrationTester();
    
    bool run_integration_test(const std::string& test_name,
                             std::function<bool()> test_function);

private:
};

// Test Suite Manager
class TestSuiteManager {
public:
    TestSuiteManager();
    ~TestSuiteManager();
    
    void add_test(const std::string& name, std::function<bool()> test_function);
    bool run_all_tests();
    bool run_test(const std::string& name);
    
    std::vector<std::string> get_test_names() const;

private:
    std::map<std::string, std::function<bool()>> tests_;
};

// Test Utilities
class TestUtils {
public:
    static std::string generate_test_packet(uint32_t src_ip, uint32_t dst_ip, 
                                          uint16_t src_port, uint16_t dst_port,
                                          uint8_t protocol, uint32_t payload_size);
    
    static bool save_packet_to_file(const std::string& filename, const std::string& packet_data);
    static std::string load_packet_from_file(const std::string& filename);
    
    static bool create_test_interface(const std::string& interface_name);
    static bool delete_test_interface(const std::string& interface_name);
    
    static std::vector<std::string> get_available_interfaces();
    static bool is_interface_up(const std::string& interface_name);
    
    static std::string format_ip_address(uint32_t ip);
    static uint32_t parse_ip_address(const std::string& ip_str);
    
    static std::string format_mac_address(const uint8_t* mac);
    static bool parse_mac_address(const std::string& mac_str, uint8_t* mac);
    
    static uint16_t calculate_checksum(const uint8_t* data, uint32_t length);
    static bool verify_checksum(const uint8_t* data, uint32_t length);
    
    static std::string generate_random_data(uint32_t size);
    static bool compare_data(const uint8_t* data1, const uint8_t* data2, uint32_t length);
    
    static uint64_t get_timestamp_us();
    static void sleep_ms(uint32_t milliseconds);
    static void sleep_us(uint32_t microseconds);
};

// Test Assertions
class TestAssertions {
public:
    static bool assert_true(bool condition, const std::string& message = "");
    static bool assert_false(bool condition, const std::string& message = "");
    static bool assert_equals(int expected, int actual, const std::string& message = "");
    static bool assert_equals(uint32_t expected, uint32_t actual, const std::string& message = "");
    static bool assert_equals(const std::string& expected, const std::string& actual, 
                             const std::string& message = "");
    static bool assert_not_equals(int expected, int actual, const std::string& message = "");
    static bool assert_greater(int value, int threshold, const std::string& message = "");
    static bool assert_less(int value, int threshold, const std::string& message = "");
    static bool assert_greater_equal(int value, int threshold, const std::string& message = "");
    static bool assert_less_equal(int value, int threshold, const std::string& message = "");
    static bool assert_not_null(const void* pointer, const std::string& message = "");
    static bool assert_null(const void* pointer, const std::string& message = "");
    static bool assert_throws(std::function<void()> function, const std::string& message = "");
    static bool assert_no_throw(std::function<void()> function, const std::string& message = "");
    
    static void fail(const std::string& message = "");
    static void skip(const std::string& message = "");
};

// Mock Objects
template<typename T>
class MockObject {
public:
    MockObject() : call_count_(0) {}
    
    void set_return_value(const T& value) {
        return_value_ = value;
    }
    
    T call() {
        call_count_++;
        return return_value_;
    }
    
    uint32_t get_call_count() const {
        return call_count_;
    }
    
    void reset() {
        call_count_ = 0;
    }

private:
    T return_value_;
    uint32_t call_count_;
};

// Test Fixtures
template<typename T>
class TestFixture {
public:
    virtual void SetUp() {}
    virtual void TearDown() {}
    
    T* get_test_object() {
        return test_object_.get();
    }
    
protected:
    std::unique_ptr<T> test_object_;
};

// Test Case Base Class
class TestCase {
public:
    TestCase(const std::string& name) : name_(name) {}
    virtual ~TestCase() = default;
    
    virtual bool run() = 0;
    const std::string& get_name() const { return name_; }

protected:
    std::string name_;
};

// Test Registry
class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry instance;
        return instance;
    }
    
    void register_test(std::unique_ptr<TestCase> test_case) {
        tests_.push_back(std::move(test_case));
    }
    
    std::vector<TestCase*> get_tests() const {
        std::vector<TestCase*> result;
        for (const auto& test : tests_) {
            result.push_back(test.get());
        }
        return result;
    }
    
    void clear() {
        tests_.clear();
    }

private:
    std::vector<std::unique_ptr<TestCase>> tests_;
};

// Test Runner
class TestRunner {
public:
    TestRunner();
    ~TestRunner();
    
    bool run_all_tests();
    bool run_test(const std::string& test_name);
    bool run_tests_by_tag(const std::string& tag);
    
    void set_verbose(bool verbose) { verbose_ = verbose; }
    void set_parallel(bool parallel) { parallel_ = parallel; }
    void set_max_threads(uint32_t max_threads) { max_threads_ = max_threads; }
    
    struct TestResult {
        std::string test_name;
        bool passed;
        uint64_t duration_ms;
        std::string error_message;
        
        TestResult() : passed(false), duration_ms(0) {}
    };
    
    std::vector<TestResult> get_results() const { return results_; }

private:
    bool verbose_;
    bool parallel_;
    uint32_t max_threads_;
    std::vector<TestResult> results_;
};

// Macro definitions for easier testing
#define TEST_CASE(name) \
    class name##TestCase : public router_sim::testing::TestCase { \
    public: \
        name##TestCase() : TestCase(#name) {} \
        bool run() override; \
    }; \
    static auto name##TestCaseInstance = []() { \
        router_sim::testing::TestRegistry::instance().register_test( \
            std::make_unique<name##TestCase>()); \
        return true; \
    }(); \
    bool name##TestCase::run()

#define ASSERT_TRUE(condition) \
    if (!router_sim::testing::TestAssertions::assert_true(condition, #condition)) { \
        return false; \
    }

#define ASSERT_FALSE(condition) \
    if (!router_sim::testing::TestAssertions::assert_false(condition, #condition)) { \
        return false; \
    }

#define ASSERT_EQ(expected, actual) \
    if (!router_sim::testing::TestAssertions::assert_equals(expected, actual, #expected " == " #actual)) { \
        return false; \
    }

#define ASSERT_NE(expected, actual) \
    if (!router_sim::testing::TestAssertions::assert_not_equals(expected, actual, #expected " != " #actual)) { \
        return false; \
    }

#define ASSERT_GT(value, threshold) \
    if (!router_sim::testing::TestAssertions::assert_greater(value, threshold, #value " > " #threshold)) { \
        return false; \
    }

#define ASSERT_LT(value, threshold) \
    if (!router_sim::testing::TestAssertions::assert_less(value, threshold, #value " < " #threshold)) { \
        return false; \
    }

#define ASSERT_GE(value, threshold) \
    if (!router_sim::testing::TestAssertions::assert_greater_equal(value, threshold, #value " >= " #threshold)) { \
        return false; \
    }

#define ASSERT_LE(value, threshold) \
    if (!router_sim::testing::TestAssertions::assert_less_equal(value, threshold, #value " <= " #threshold)) { \
        return false; \
    }

#define ASSERT_NOT_NULL(pointer) \
    if (!router_sim::testing::TestAssertions::assert_not_null(pointer, #pointer " != nullptr")) { \
        return false; \
    }

#define ASSERT_NULL(pointer) \
    if (!router_sim::testing::TestAssertions::assert_null(pointer, #pointer " == nullptr")) { \
        return false; \
    }

#define ASSERT_THROWS(function) \
    if (!router_sim::testing::TestAssertions::assert_throws(function, #function " should throw")) { \
        return false; \
    }

#define ASSERT_NO_THROW(function) \
    if (!router_sim::testing::TestAssertions::assert_no_throw(function, #function " should not throw")) { \
        return false; \
    }

#define FAIL(message) \
    router_sim::testing::TestAssertions::fail(message); \
    return false;

#define SKIP(message) \
    router_sim::testing::TestAssertions::skip(message); \
    return true;

} // namespace testing
} // namespace router_sim
