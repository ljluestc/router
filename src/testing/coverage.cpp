#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

namespace RouterSim {

class CoverageAnalyzer {
public:
    CoverageAnalyzer() : total_lines_(0), covered_lines_(0) {}
    
    void add_file(const std::string& filename, const std::vector<int>& line_numbers) {
        file_coverage_[filename] = line_numbers;
        total_lines_ += line_numbers.size();
    }
    
    void mark_line_executed(const std::string& filename, int line_number) {
        auto it = file_coverage_.find(filename);
        if (it != file_coverage_.end()) {
            auto& lines = it->second;
            auto line_it = std::find(lines.begin(), lines.end(), line_number);
            if (line_it != lines.end()) {
                executed_lines_[filename].insert(line_number);
                covered_lines_++;
            }
        }
    }
    
    double get_coverage_percentage() const {
        if (total_lines_ == 0) {
            return 0.0;
        }
        return (static_cast<double>(covered_lines_) / total_lines_) * 100.0;
    }
    
    void generate_report(const std::string& output_file) {
        std::ofstream file(output_file);
        if (!file.is_open()) {
            std::cerr << "Failed to open output file: " << output_file << std::endl;
            return;
        }
        
        file << "Code Coverage Report" << std::endl;
        file << "Generated: " << get_current_timestamp() << std::endl;
        file << "======================================" << std::endl;
        file << std::endl;
        
        file << "Overall Coverage: " << std::fixed << std::setprecision(2) 
             << get_coverage_percentage() << "%" << std::endl;
        file << "Total Lines: " << total_lines_ << std::endl;
        file << "Covered Lines: " << covered_lines_ << std::endl;
        file << "Uncovered Lines: " << (total_lines_ - covered_lines_) << std::endl;
        file << std::endl;
        
        // Per-file coverage
        file << "Per-File Coverage:" << std::endl;
        file << "==================" << std::endl;
        
        for (const auto& file_pair : file_coverage_) {
            const std::string& filename = file_pair.first;
            const std::vector<int>& lines = file_pair.second;
            
            int covered = 0;
            auto executed_it = executed_lines_.find(filename);
            if (executed_it != executed_lines_.end()) {
                covered = executed_it->second.size();
            }
            
            double percentage = (static_cast<double>(covered) / lines.size()) * 100.0;
            
            file << filename << ": " << std::fixed << std::setprecision(2) 
                 << percentage << "% (" << covered << "/" << lines.size() << ")" << std::endl;
        }
        
        file << std::endl;
        
        // Detailed coverage for each file
        file << "Detailed Coverage:" << std::endl;
        file << "==================" << std::endl;
        
        for (const auto& file_pair : file_coverage_) {
            const std::string& filename = file_pair.first;
            const std::vector<int>& lines = file_pair.second;
            
            file << std::endl << "File: " << filename << std::endl;
            file << "Lines: ";
            
            for (int line : lines) {
                bool is_covered = false;
                auto executed_it = executed_lines_.find(filename);
                if (executed_it != executed_lines_.end()) {
                    is_covered = executed_it->second.count(line) > 0;
                }
                
                file << line << (is_covered ? "+" : "-") << " ";
            }
            file << std::endl;
        }
        
        file.close();
        std::cout << "Coverage report generated: " << output_file << std::endl;
    }
    
    void print_summary() const {
        std::cout << "Code Coverage Summary:" << std::endl;
        std::cout << "  Overall Coverage: " << std::fixed << std::setprecision(2) 
                  << get_coverage_percentage() << "%" << std::endl;
        std::cout << "  Total Lines: " << total_lines_ << std::endl;
        std::cout << "  Covered Lines: " << covered_lines_ << std::endl;
        std::cout << "  Uncovered Lines: " << (total_lines_ - covered_lines_) << std::endl;
    }

private:
    std::map<std::string, std::vector<int>> file_coverage_;
    std::map<std::string, std::set<int>> executed_lines_;
    int total_lines_;
    int covered_lines_;
    
    std::string get_current_timestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// Global coverage analyzer instance
static CoverageAnalyzer g_coverage_analyzer;

// Coverage tracking macros
#define COVERAGE_TRACK(filename, line) \
    g_coverage_analyzer.mark_line_executed(filename, line)

#define COVERAGE_ADD_FILE(filename, lines) \
    g_coverage_analyzer.add_file(filename, lines)

#define COVERAGE_GENERATE_REPORT(output_file) \
    g_coverage_analyzer.generate_report(output_file)

#define COVERAGE_PRINT_SUMMARY() \
    g_coverage_analyzer.print_summary()

} // namespace RouterSim
