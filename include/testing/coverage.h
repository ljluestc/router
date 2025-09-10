#pragma once

#include <string>
#include <map>
#include <mutex>
#include <filesystem>

namespace RouterSim {

// Coverage statistics
struct CoverageStats {
    uint64_t total_lines;
    uint64_t total_lines_executed;
    uint64_t total_branches;
    uint64_t total_branches_executed;
    uint64_t total_functions;
    uint64_t total_functions_called;
    uint64_t total_function_calls;
    uint64_t total_conditions;
    
    void reset() {
        total_lines = 0;
        total_lines_executed = 0;
        total_branches = 0;
        total_branches_executed = 0;
        total_functions = 0;
        total_functions_called = 0;
        total_function_calls = 0;
        total_conditions = 0;
    }
};

// Coverage tracker class
class CoverageTracker {
public:
    CoverageTracker();
    ~CoverageTracker();
    
    // Lifecycle
    bool initialize();
    bool start();
    bool stop();
    bool is_enabled() const;
    
    // Coverage recording
    void record_function_call(const std::string& function_name);
    void record_branch_taken(const std::string& function_name, int branch_id, bool taken);
    void record_line_executed(const std::string& file_name, int line_number);
    void record_condition_evaluated(const std::string& function_name, int condition_id, bool result);
    
    // Statistics retrieval
    CoverageStats get_statistics() const;
    std::map<std::string, uint64_t> get_function_coverage() const;
    std::map<std::string, uint64_t> get_line_coverage() const;
    std::map<std::string, uint64_t> get_branch_coverage() const;
    std::map<std::string, uint64_t> get_condition_coverage() const;
    
    // Coverage calculations
    double calculate_line_coverage_percentage() const;
    double calculate_branch_coverage_percentage() const;
    double calculate_function_coverage_percentage() const;
    
    // Report generation
    bool generate_coverage_report(const std::string& output_dir = "coverage_report");
    
    // Configuration
    void set_total_lines(uint64_t total_lines);
    void set_total_functions(uint64_t total_functions);
    void set_total_branches(uint64_t total_branches);
    
    // Validation
    bool meets_coverage_threshold(double threshold) const;
    
private:
    // Internal methods
    void generate_text_report(const std::string& output_dir);
    void reset_coverage_data();
    
    // State
    bool initialized_;
    bool enabled_;
    
    // Coverage data
    std::map<std::string, uint64_t> function_calls_;
    std::map<std::string, uint64_t> executed_lines_;
    std::map<std::string, uint64_t> branches_taken_;
    std::map<std::string, uint64_t> branches_not_taken_;
    std::map<std::string, uint64_t> conditions_true_;
    std::map<std::string, uint64_t> conditions_false_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    CoverageStats stats_;
};

} // namespace RouterSim
