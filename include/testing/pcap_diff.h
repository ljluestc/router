#pragma once

#include <string>
#include <vector>
#include <regex>

namespace RouterSim {

// Pcap diff options
struct PcapDiffOptions {
    bool verbose;
    bool ignore_timestamps;
    std::string filter;
    
    PcapDiffOptions() : verbose(false), ignore_timestamps(true) {}
};

// Pcap validation options
struct PcapValidationOptions {
    size_t min_size;
    int min_packets;
    std::vector<std::string> required_protocols;
    
    PcapValidationOptions() : min_size(0), min_packets(0) {}
};

// Pcap difference information
struct PcapDifference {
    size_t line_number;
    std::string type; // "ADDED", "REMOVED", "MODIFIED"
    std::string expected;
    std::string actual;
};

// Pcap diff result
struct PcapDiffResult {
    bool identical;
    double similarity;
    std::vector<PcapDifference> differences;
    std::string diff_output;
    std::string error_message;
    
    PcapDiffResult() : identical(false), similarity(0.0) {}
};

// Pcap diff utility class
class PcapDiff {
public:
    PcapDiff();
    ~PcapDiff();
    
    // Initialization
    bool initialize();
    
    // Basic comparison
    bool compare_pcap_files(const std::string& file1, const std::string& file2, 
                           const PcapDiffOptions& options = PcapDiffOptions());
    
    // Detailed comparison
    bool compare_pcap_files_detailed(const std::string& file1, const std::string& file2,
                                    const PcapDiffOptions& options, PcapDiffResult& result);
    
    // Validation
    bool validate_pcap_file(const std::string& filename, const PcapValidationOptions& options = PcapValidationOptions());
    
private:
    // Text conversion
    std::string pcap_to_text(const std::string& filename, const PcapDiffOptions& options);
    
    // Text comparison
    bool compare_text(const std::string& text1, const std::string& text2, const PcapDiffOptions& options);
    bool compare_text_detailed(const std::string& text1, const std::string& text2,
                              const PcapDiffOptions& options, PcapDiffResult& result);
    
    // Text processing
    std::string remove_timestamps(const std::string& text);
    std::vector<PcapDifference> calculate_differences(const std::string& text1, const std::string& text2);
    double calculate_similarity(const std::string& text1, const std::string& text2);
    std::string generate_diff_output(const std::string& text1, const std::string& text2);
    
    // Utility functions
    bool file_exists(const std::string& filename);
    int count_packets(const std::string& filename);
    bool has_protocol(const std::string& filename, const std::string& protocol);
    std::vector<std::string> split_lines(const std::string& text);
    
    bool initialized_;
};

} // namespace RouterSim
