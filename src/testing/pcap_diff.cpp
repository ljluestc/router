#include "testing/pcap_diff.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace RouterSim {

PcapDiff::PcapDiff() : initialized_(false) {
}

PcapDiff::~PcapDiff() = default;

bool PcapDiff::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Check if required tools are available
    int result = std::system("which tcpdump > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "tcpdump not found. Please install tcpdump." << std::endl;
        return false;
    }
    
    result = std::system("which diff > /dev/null 2>&1");
    if (result != 0) {
        std::cerr << "diff not found. Please install diffutils." << std::endl;
        return false;
    }
    
    initialized_ = true;
    std::cout << "PcapDiff initialized successfully" << std::endl;
    return true;
}

bool PcapDiff::compare_pcap_files(const std::string& file1, const std::string& file2, 
                                 const PcapDiffOptions& options) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    
    // Check if files exist
    if (!file_exists(file1) || !file_exists(file2)) {
        std::cerr << "One or both pcap files do not exist" << std::endl;
        return false;
    }
    
    // Generate text representations of pcap files
    std::string text1 = pcap_to_text(file1, options);
    std::string text2 = pcap_to_text(file2, options);
    
    if (text1.empty() || text2.empty()) {
        std::cerr << "Failed to convert pcap files to text" << std::endl;
        return false;
    }
    
    // Compare the text representations
    return compare_text(text1, text2, options);
}

bool PcapDiff::compare_pcap_files_detailed(const std::string& file1, const std::string& file2,
                                          const PcapDiffOptions& options, PcapDiffResult& result) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    // Check if files exist
    if (!file_exists(file1) || !file_exists(file2)) {
        result.error_message = "One or both pcap files do not exist";
        return false;
    }
    
    // Generate text representations
    std::string text1 = pcap_to_text(file1, options);
    std::string text2 = pcap_to_text(file2, options);
    
    if (text1.empty() || text2.empty()) {
        result.error_message = "Failed to convert pcap files to text";
        return false;
    }
    
    // Compare with detailed results
    return compare_text_detailed(text1, text2, options, result);
}

bool PcapDiff::validate_pcap_file(const std::string& filename, const PcapValidationOptions& options) {
    if (!initialized_) {
        if (!initialize()) {
            return false;
        }
    }
    
    if (!file_exists(filename)) {
        std::cerr << "Pcap file does not exist: " << filename << std::endl;
        return false;
    }
    
    // Check file size
    if (options.min_size > 0) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streampos size = file.tellg();
            if (size < options.min_size) {
                std::cerr << "Pcap file too small: " << size << " bytes (minimum: " << options.min_size << ")" << std::endl;
                return false;
            }
        }
    }
    
    // Check packet count
    if (options.min_packets > 0) {
        int packet_count = count_packets(filename);
        if (packet_count < options.min_packets) {
            std::cerr << "Pcap file has too few packets: " << packet_count << " (minimum: " << options.min_packets << ")" << std::endl;
            return false;
        }
    }
    
    // Check for specific protocols
    if (!options.required_protocols.empty()) {
        for (const auto& protocol : options.required_protocols) {
            if (!has_protocol(filename, protocol)) {
                std::cerr << "Pcap file missing required protocol: " << protocol << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

std::string PcapDiff::pcap_to_text(const std::string& filename, const PcapDiffOptions& options) {
    std::ostringstream cmd;
    cmd << "tcpdump -r " << filename;
    
    if (options.verbose) {
        cmd << " -v";
    }
    
    if (!options.filter.empty()) {
        cmd << " " << options.filter;
    }
    
    cmd << " 2>/dev/null";
    
    // Execute command and capture output
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    return result;
}

bool PcapDiff::compare_text(const std::string& text1, const std::string& text2, 
                           const PcapDiffOptions& options) {
    if (options.ignore_timestamps) {
        // Remove timestamps from both texts
        std::string clean_text1 = remove_timestamps(text1);
        std::string clean_text2 = remove_timestamps(text2);
        return clean_text1 == clean_text2;
    }
    
    return text1 == text2;
}

bool PcapDiff::compare_text_detailed(const std::string& text1, const std::string& text2,
                                    const PcapDiffOptions& options, PcapDiffResult& result) {
    result.identical = (text1 == text2);
    
    if (result.identical) {
        return true;
    }
    
    // Calculate differences
    result.differences = calculate_differences(text1, text2);
    result.similarity = calculate_similarity(text1, text2);
    
    // Generate diff output
    result.diff_output = generate_diff_output(text1, text2);
    
    return false;
}

std::string PcapDiff::remove_timestamps(const std::string& text) {
    std::string result = text;
    
    // Remove timestamp patterns like "12:34:56.789123"
    std::regex timestamp_regex(R"(\d{2}:\d{2}:\d{2}\.\d{6})");
    result = std::regex_replace(result, timestamp_regex, "XX:XX:XX.XXXXXX");
    
    return result;
}

std::vector<PcapDifference> PcapDiff::calculate_differences(const std::string& text1, const std::string& text2) {
    std::vector<PcapDifference> differences;
    
    // Split texts into lines
    std::vector<std::string> lines1 = split_lines(text1);
    std::vector<std::string> lines2 = split_lines(text2);
    
    // Find differences using simple line-by-line comparison
    size_t max_lines = std::max(lines1.size(), lines2.size());
    
    for (size_t i = 0; i < max_lines; ++i) {
        std::string line1 = (i < lines1.size()) ? lines1[i] : "";
        std::string line2 = (i < lines2.size()) ? lines2[i] : "";
        
        if (line1 != line2) {
            PcapDifference diff;
            diff.line_number = i + 1;
            diff.type = (i >= lines1.size()) ? "ADDED" : (i >= lines2.size()) ? "REMOVED" : "MODIFIED";
            diff.expected = line1;
            diff.actual = line2;
            differences.push_back(diff);
        }
    }
    
    return differences;
}

double PcapDiff::calculate_similarity(const std::string& text1, const std::string& text2) {
    if (text1.empty() && text2.empty()) {
        return 1.0;
    }
    
    if (text1.empty() || text2.empty()) {
        return 0.0;
    }
    
    // Simple similarity calculation based on common substrings
    size_t common_chars = 0;
    size_t min_length = std::min(text1.length(), text2.length());
    
    for (size_t i = 0; i < min_length; ++i) {
        if (text1[i] == text2[i]) {
            common_chars++;
        }
    }
    
    return static_cast<double>(common_chars) / std::max(text1.length(), text2.length());
}

std::string PcapDiff::generate_diff_output(const std::string& text1, const std::string& text2) {
    // Write texts to temporary files
    std::string temp1 = "/tmp/pcap_diff_1.txt";
    std::string temp2 = "/tmp/pcap_diff_2.txt";
    
    std::ofstream file1(temp1);
    std::ofstream file2(temp2);
    
    if (!file1.is_open() || !file2.is_open()) {
        return "Failed to create temporary files";
    }
    
    file1 << text1;
    file2 << text2;
    
    file1.close();
    file2.close();
    
    // Run diff command
    std::ostringstream cmd;
    cmd << "diff -u " << temp1 << " " << temp2 << " 2>/dev/null";
    
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return "Failed to run diff command";
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    
    // Clean up temporary files
    std::remove(temp1.c_str());
    std::remove(temp2.c_str());
    
    return result;
}

bool PcapDiff::file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

int PcapDiff::count_packets(const std::string& filename) {
    std::ostringstream cmd;
    cmd << "tcpdump -r " << filename << " 2>/dev/null | wc -l";
    
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return -1;
    }
    
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        pclose(pipe);
        return std::stoi(buffer);
    }
    
    pclose(pipe);
    return -1;
}

bool PcapDiff::has_protocol(const std::string& filename, const std::string& protocol) {
    std::ostringstream cmd;
    cmd << "tcpdump -r " << filename << " " << protocol << " 2>/dev/null | head -1";
    
    FILE* pipe = popen(cmd.str().c_str(), "r");
    if (!pipe) {
        return false;
    }
    
    char buffer[128];
    bool has_protocol = (fgets(buffer, sizeof(buffer), pipe) != nullptr);
    
    pclose(pipe);
    return has_protocol;
}

std::vector<std::string> PcapDiff::split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

} // namespace RouterSim