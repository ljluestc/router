#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace RouterSim {

// YAML configuration parser
class YAMLConfig {
public:
    YAMLConfig();
    ~YAMLConfig();
    
    // Core functionality
    bool load(const std::string& filename);
    bool save(const std::string& filename) const;
    void clear();
    
    // Value access
    std::string get_string(const std::string& key, const std::string& default_value = "") const;
    int get_int(const std::string& key, int default_value = 0) const;
    double get_double(const std::string& key, double default_value = 0.0) const;
    bool get_bool(const std::string& key, bool default_value = false) const;
    
    // Value setting
    void set_string(const std::string& key, const std::string& value);
    void set_int(const std::string& key, int value);
    void set_double(const std::string& key, double value);
    void set_bool(const std::string& key, bool value);
    
    // Array access
    std::vector<std::string> get_string_array(const std::string& key) const;
    std::vector<int> get_int_array(const std::string& key) const;
    void set_string_array(const std::string& key, const std::vector<std::string>& values);
    void set_int_array(const std::string& key, const std::vector<int>& values);
    
    // Section access
    std::map<std::string, std::string> get_section(const std::string& section) const;
    void set_section(const std::string& section, const std::map<std::string, std::string>& values);
    
    // Validation
    bool has_key(const std::string& key) const;
    std::vector<std::string> get_keys() const;
    
    // Raw access
    std::string get_raw_yaml() const;
    void set_raw_yaml(const std::string& yaml_content);

private:
    void* yaml_document_; // Opaque pointer for YAML document
    std::map<std::string, std::string> config_map_;
    
    // Internal methods
    std::string parse_key_path(const std::string& key) const;
    void parse_yaml_to_map();
    std::string map_to_yaml() const;
};

} // namespace RouterSim