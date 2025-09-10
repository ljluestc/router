#include "testing/coverage.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <filesystem>

namespace RouterSim {

CoverageTracker::CoverageTracker() : initialized_(false), enabled_(false) {
    stats_.reset();
}

CoverageTracker::~CoverageTracker() {
    if (enabled_) {
        stop();
    }
}

bool CoverageTracker::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Check if gcov is available
    int result = std::system("which gcov > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "gcov not found. Please install gcc with coverage support." << std::endl;
        return false;
    }
    
    // Check if lcov is available
    result = std::system("which lcov > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "lcov not found. Please install lcov." << std::endl;
        return false;
    }
    
    initialized_ = true;
    std::cout << "CoverageTracker initialized successfully" << std::endl;
    return true;
}

bool CoverageTracker::start() {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    if (enabled_) {
        return true;
    }
    
    // Reset coverage data
    reset_coverage_data();
    
    enabled_ = true;
    std::cout << "Coverage tracking started" << std::endl;
    return true;
}

bool CoverageTracker::stop() {
    if (!enabled_) {
        return true;
    }
    
    // Generate coverage report
    generate_coverage_report();
    
    enabled_ = false;
    std::cout << "Coverage tracking stopped" << std::endl;
    return true;
}

bool CoverageTracker::is_enabled() const {
    return enabled_;
}

void CoverageTracker::record_function_call(const std::string& function_name) {
    if (!enabled_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    auto it = function_calls_.find(function_name);
    if (it != function_calls_.end()) {
        it->second++;
    } else {
        function_calls_[function_name] = 1;
    }
    
    stats_.total_function_calls++;
}

void CoverageTracker::record_branch_taken(const std::string& function_name, int branch_id, bool taken) {
    if (!enabled_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::string key = function_name + ":" + std::to_string(branch_id);
    
    if (taken) {
        branches_taken_[key]++;
    } else {
        branches_not_taken_[key]++;
    }
    
    stats_.total_branches++;
}

void CoverageTracker::record_line_executed(const std::string& file_name, int line_number) {
    if (!enabled_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::string key = file_name + ":" + std::to_string(line_number);
    executed_lines_[key]++;
    
    stats_.total_lines_executed++;
}

void CoverageTracker::record_condition_evaluated(const std::string& function_name, int condition_id, bool result) {
    if (!enabled_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    std::string key = function_name + ":" + std::to_string(condition_id);
    
    if (result) {
        conditions_true_[key]++;
    } else {
        conditions_false_[key]++;
    }
    
    stats_.total_conditions++;
}

CoverageStats CoverageTracker::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

std::map<std::string, uint64_t> CoverageTracker::get_function_coverage() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return function_calls_;
}

std::map<std::string, uint64_t> CoverageTracker::get_line_coverage() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return executed_lines_;
}

std::map<std::string, uint64_t> CoverageTracker::get_branch_coverage() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    std::map<std::string, uint64_t> branch_coverage;
    
    for (const auto& [key, count] : branches_taken_) {
        branch_coverage[key + ":taken"] = count;
    }
    
    for (const auto& [key, count] : branches_not_taken_) {
        branch_coverage[key + ":not_taken"] = count;
    }
    
    return branch_coverage;
}

std::map<std::string, uint64_t> CoverageTracker::get_condition_coverage() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    std::map<std::string, uint64_t> condition_coverage;
    
    for (const auto& [key, count] : conditions_true_) {
        condition_coverage[key + ":true"] = count;
    }
    
    for (const auto& [key, count] : conditions_false_) {
        condition_coverage[key + ":false"] = count;
    }
    
    return condition_coverage;
}

double CoverageTracker::calculate_line_coverage_percentage() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (stats_.total_lines == 0) {
        return 0.0;
    }
    
    return (static_cast<double>(stats_.total_lines_executed) / stats_.total_lines) * 100.0;
}

double CoverageTracker::calculate_branch_coverage_percentage() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (stats_.total_branches == 0) {
        return 0.0;
    }
    
    return (static_cast<double>(stats_.total_branches_executed) / stats_.total_branches) * 100.0;
}

double CoverageTracker::calculate_function_coverage_percentage() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (stats_.total_functions == 0) {
        return 0.0;
    }
    
    return (static_cast<double>(stats_.total_functions_called) / stats_.total_functions) * 100.0;
}

bool CoverageTracker::generate_coverage_report(const std::string& output_dir) {
    if (!enabled_) {
        return false;
    }
    
    // Create output directory
    std::filesystem::create_directories(output_dir);
    
    // Generate gcov reports
    std::ostringstream cmd;
    cmd << "find . -name '*.gcno' -exec gcov {} \\; > /dev/null 2>&1";
    std::system(cmd.str().c_str());
    
    // Generate lcov report
    cmd.str("");
    cmd << "lcov --capture --directory . --output-file " << output_dir << "/coverage.info";
    int result = std::system(cmd.str().c_str());
    
    if (result != 0) {
        std::cerr << "Failed to generate lcov report" << std::endl;
        return false;
    }
    
    // Generate HTML report
    cmd.str("");
    cmd << "genhtml " << output_dir << "/coverage.info --output-directory " << output_dir << "/html";
    result = std::system(cmd.str().c_str());
    
    if (result != 0) {
        std::cerr << "Failed to generate HTML report" << std::endl;
        return false;
    }
    
    // Generate text report
    generate_text_report(output_dir);
    
    std::cout << "Coverage report generated in " << output_dir << std::endl;
    return true;
}

void CoverageTracker::generate_text_report(const std::string& output_dir) {
    std::string filename = output_dir + "/coverage_summary.txt";
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Failed to create coverage summary file" << std::endl;
        return;
    }
    
    file << "Coverage Summary Report\n";
    file << "======================\n\n";
    
    file << "Line Coverage: " << std::fixed << std::setprecision(2) 
         << calculate_line_coverage_percentage() << "%\n";
    file << "Branch Coverage: " << std::fixed << std::setprecision(2) 
         << calculate_branch_coverage_percentage() << "%\n";
    file << "Function Coverage: " << std::fixed << std::setprecision(2) 
         << calculate_function_coverage_percentage() << "%\n\n";
    
    file << "Statistics:\n";
    file << "  Total Lines: " << stats_.total_lines << "\n";
    file << "  Lines Executed: " << stats_.total_lines_executed << "\n";
    file << "  Total Branches: " << stats_.total_branches << "\n";
    file << "  Branches Executed: " << stats_.total_branches_executed << "\n";
    file << "  Total Functions: " << stats_.total_functions << "\n";
    file << "  Functions Called: " << stats_.total_functions_called << "\n";
    file << "  Total Function Calls: " << stats_.total_function_calls << "\n";
    file << "  Total Conditions: " << stats_.total_conditions << "\n\n";
    
    file << "Function Coverage Details:\n";
    file << "=========================\n";
    for (const auto& [function, count] : function_calls_) {
        file << "  " << function << ": " << count << " calls\n";
    }
    
    file.close();
}

void CoverageTracker::reset_coverage_data() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    function_calls_.clear();
    executed_lines_.clear();
    branches_taken_.clear();
    branches_not_taken_.clear();
    conditions_true_.clear();
    conditions_false_.clear();
    
    stats_.reset();
}

void CoverageTracker::set_total_lines(uint64_t total_lines) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_lines = total_lines;
}

void CoverageTracker::set_total_functions(uint64_t total_functions) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_functions = total_functions;
}

void CoverageTracker::set_total_branches(uint64_t total_branches) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_branches = total_branches;
}

bool CoverageTracker::meets_coverage_threshold(double threshold) const {
    double line_coverage = calculate_line_coverage_percentage();
    double branch_coverage = calculate_branch_coverage_percentage();
    double function_coverage = calculate_function_coverage_percentage();
    
    return (line_coverage >= threshold) && (branch_coverage >= threshold) && (function_coverage >= threshold);
}

} // namespace RouterSim
