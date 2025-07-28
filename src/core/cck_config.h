/**
 * @file cck_config.h
 * @brief Universal configuration management for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 * 
 * This header provides a centralized configuration management system for ComChemKit.
 * It handles loading, parsing, and accessing configuration values from various sources:
 * - Default values from cck_constants.h
 * - System-wide configuration file (/etc/cck/config)
 * - User configuration file (~/.cck.conf)
 * - Local configuration file (./.cck.conf)
 * - Command-line overrides
 * 
 * Configuration values are accessible through a thread-safe singleton interface
 * that ensures consistent configuration across all program modules.
 */

#ifndef CCK_CONFIG_H
#define CCK_CONFIG_H

#include "cck_constants.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include <filesystem>

namespace cck {
namespace core {

/**
 * @brief Configuration value types supported by the system
 */
enum class ConfigValueType {
    STRING,
    INTEGER,
    DOUBLE,
    BOOLEAN,
    PATH,
    STRING_LIST
};

/**
 * @brief Configuration value with type information and validation
 */
class ConfigValue {
public:
    // Constructors for different value types
    explicit ConfigValue(const std::string& str) : type_(ConfigValueType::STRING), str_value_(str) {}
    explicit ConfigValue(int i) : type_(ConfigValueType::INTEGER), int_value_(i) {}
    explicit ConfigValue(double d) : type_(ConfigValueType::DOUBLE), double_value_(d) {}
    explicit ConfigValue(bool b) : type_(ConfigValueType::BOOLEAN), bool_value_(b) {}
    explicit ConfigValue(const std::filesystem::path& p) : type_(ConfigValueType::PATH), path_value_(p) {}
    explicit ConfigValue(const std::vector<std::string>& list) 
        : type_(ConfigValueType::STRING_LIST), list_value_(list) {}

    // Value accessors with type checking
    std::string as_string() const;
    int as_integer() const;
    double as_double() const;
    bool as_boolean() const;
    std::filesystem::path as_path() const;
    std::vector<std::string> as_string_list() const;

    // Type checking
    ConfigValueType get_type() const { return type_; }
    bool is_type(ConfigValueType t) const { return type_ == t; }

private:
    ConfigValueType type_;
    std::string str_value_;
    int int_value_ = 0;
    double double_value_ = 0.0;
    bool bool_value_ = false;
    std::filesystem::path path_value_;
    std::vector<std::string> list_value_;
};

/**
 * @brief Thread-safe configuration manager singleton
 */
class Config {
public:
    // Singleton access
    static Config& get_instance();

    // Delete copy and move operations
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // Configuration loading
    bool load_config(const std::string& config_file = "");
    void set_command_line_overrides(const std::unordered_map<std::string, std::string>& overrides);

    // Value access
    std::optional<ConfigValue> get_value(const std::string& key) const;
    std::string get_string(const std::string& key, const std::string& default_value = "") const;
    int get_integer(const std::string& key, int default_value = 0) const;
    double get_double(const std::string& key, double default_value = 0.0) const;
    bool get_boolean(const std::string& key, bool default_value = false) const;
    std::filesystem::path get_path(const std::string& key, const std::filesystem::path& default_value = "") const;
    std::vector<std::string> get_string_list(const std::string& key, 
                                           const std::vector<std::string>& default_value = {}) const;

    // Configuration validation and information
    bool validate_config() const;
    std::vector<std::string> get_validation_errors() const;
    std::vector<std::string> get_available_keys() const;
    bool has_key(const std::string& key) const;

    // Default configuration management
    void create_default_config(const std::string& filename) const;
    static std::string get_config_help();

private:
    Config(); // Private constructor for singleton

    bool load_file(const std::string& filename);
    bool parse_config_line(const std::string& line);
    void apply_defaults();
    void merge_command_line_overrides();
    bool validate_value_type(const std::string& key, ConfigValueType expected_type) const;

    static std::string find_config_file();
    static std::vector<std::string> get_config_search_paths();

    mutable std::mutex mutex_;
    std::unordered_map<std::string, ConfigValue> config_values_;
    std::unordered_map<std::string, std::string> command_line_overrides_;
    std::vector<std::string> validation_errors_;
    bool config_loaded_ = false;
};

// Global configuration accessor
inline Config& g_config() {
    return Config::get_instance();
}

} // namespace core
} // namespace cck

#endif // CCK_CONFIG_H