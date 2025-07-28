/**
 * @file cck_command_system.h
 * @brief Command line parsing and execution system for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 * 
 * This header provides the core command system infrastructure for ComChemKit.
 * It handles command-line argument parsing, command dispatch, and execution
 * context management for all supported quantum chemistry programs.
 * 
 * The command system is designed to be extensible, allowing new quantum chemistry
 * program modules to register their own commands while using the common
 * infrastructure for parsing and execution.
 * 
 * Features:
 * - Unified command-line argument parsing
 * - Program-specific command registration
 * - Execution context management
 * - Resource utilization controls
 * - Error handling and reporting
 * 
 * @section Usage
 * Each quantum chemistry program module (Gaussian, ORCA, etc.) can register
 * its commands with the command system. The system handles:
 * - Command-line parsing
 * - Option validation
 * - Resource limits
 * - Error handling
 */

#ifndef CCK_COMMAND_SYSTEM_H
#define CCK_COMMAND_SYSTEM_H

#include "cck_version.h"
#include "cck_job_scheduler.h"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include <cstdlib>

namespace cck {
namespace core {

/**
 * @brief Supported command types in ComChemKit
 */
enum class CommandType {
    NONE,           ///< No command specified
    HELP,           ///< Show help information
    VERSION,        ///< Show version information
    RESOURCE_INFO,  ///< Show system resource information
    
    // Gaussian-specific commands
    EXTRACT,        ///< Extract data from Gaussian output files
    CHECK_DONE,     ///< Check for completed Gaussian jobs
    CHECK_ERRORS,   ///< Check for Gaussian job errors
    CHECK_PCM,      ///< Check for PCM-related issues
    CHECK_ALL,      ///< Run all Gaussian checks
    HIGH_LEVEL_KJ,  ///< High-level energy calculation (kJ/mol)
    HIGH_LEVEL_AU,  ///< High-level energy calculation (atomic units)
    
    // Future command types for other programs will be added here
    // ORCA commands
    // ORCA_EXTRACT,
    // ORCA_CHECK,
    // etc.
    
    // NWChem commands
    // NWCHEM_EXTRACT,
    // NWCHEM_CHECK,
    // etc.
};

/**
 * @brief Command execution context
 * 
 * Contains all parameters and state needed for command execution
 */
struct CommandContext {
    CommandType command = CommandType::NONE;  ///< Command to execute
    std::string input_dir;                    ///< Input directory path
    double temperature = 298.15;              ///< Temperature in Kelvin
    double concentration = 1.0;               ///< Concentration in mol/L
    int thread_count = -1;                    ///< Number of threads (-1 = auto)
    int sort_column = 2;                      ///< Column to sort by
    std::string format = "text";              ///< Output format
    std::string extension = "log";            ///< File extension to process
    bool quiet = false;                       ///< Suppress non-essential output
    size_t max_file_size = 100;              ///< Maximum file size in MB
    size_t memory_limit_mb = 0;               ///< Memory limit in MB (0 = auto)
    std::vector<std::string> warnings;        ///< Accumulated warnings
    
    // Additional fields for command executor compatibility
    bool show_resource_info = false;          ///< Show system resource information
    bool use_input_temp = false;              ///< Use temperature from input files
    bool show_error_details = false;          ///< Show detailed error information
    int requested_threads = 1;                ///< Requested number of threads
    std::string target_dir;                   ///< Target directory for operations
    std::string dir_suffix;                   ///< Directory suffix for operations
    double temp = 298.15;                     ///< Temperature alias for temperature
    JobResources job_resources;               ///< Job scheduler resource information
    
    // Backward compatibility - both fields point to same value
    size_t memory_limit = 0;                  ///< Memory limit in MB (backward compatibility)


};

/**
 * @brief Get the default quantum chemistry program from configuration
 * @return Name of the default program (e.g., "gaussian", "orca")
 * 
 * Reads the default_program setting from configuration. If not found or
 * configuration is not loaded, returns "gaussian" as the fallback default.
 */
std::string get_default_program();

/**
 * @brief Command parser class
 * 
 * Handles parsing of command-line arguments and options
 */
class CommandParser {
public:
    /**
     * @brief Parse command-line arguments
     * @param argc Argument count
     * @param argv Argument array
     * @return Parsed command context
     */
    static CommandContext parse(int argc, char* argv[]);
    
    /**
     * @brief Show help message
     * @param command Optional specific command to show help for
     */
    static void show_help(const std::string& command = "");
    
private:
    /**
     * @brief Parse command string to CommandType
     * @param cmd Command string to parse
     * @return Corresponding CommandType
     */
    static CommandType parse_command(const std::string& cmd);
    /**
     * @brief Parse resource-related options
     * @param context Command context to update
     * @param args Remaining arguments to parse
     * @param pos Current position in args
     * @return New position in args
     */
    static size_t parse_resource_options(CommandContext& context, 
                                       const std::vector<std::string>& args,
                                       size_t pos);
    
    /**
     * @brief Parse format options
     * @param context Command context to update
     * @param args Remaining arguments to parse
     * @param pos Current position in args
     * @return New position in args
     */
    static size_t parse_format_options(CommandContext& context,
                                     const std::vector<std::string>& args,
                                     size_t pos);
    
    /**
     * @brief Validate parsed options
     * @param context Command context to validate
     * @return true if validation succeeds
     */
    static bool validate_options(CommandContext& context);
};

} // namespace core
} // namespace cck

#endif // CCK_COMMAND_SYSTEM_H