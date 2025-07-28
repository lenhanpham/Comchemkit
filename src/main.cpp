/**
 * @file main.cpp
 * @brief Main entry point for the Gaussian Extractor application
 * @author Le Nhan Pham
 * @date 2025
 *
 * This file contains the main function and signal handling for the Gaussian Extractor,
 * a high-performance tool for processing Gaussian computational chemistry log files.
 * The application supports various commands including extraction, job checking, and
 * high-level energy calculations.
 *
 * @section Features
 * - Multi-threaded file processing with resource management
 * - Job scheduler integration (SLURM, PBS, SGE, LSF)
 * - Comprehensive error detection and job status checking
 * - High-level energy calculations with thermal corrections
 * - Configurable through configuration files and command-line options
 * - Graceful shutdown handling for long-running operations
 */

#include "core/cck_command_system.h"
#include "core/cck_config_manager.h"
#include "gaussian/gaussian_commands.h"
#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <csignal>
#include <string>
#include <vector>
#include <exception>

/**
 * @brief Global flag to indicate when a shutdown has been requested
 *
 * This atomic boolean is used to coordinate graceful shutdown across all threads
 * when a termination signal (SIGINT, SIGTERM) is received. All long-running
 * operations should periodically check this flag and terminate cleanly.
 */
std::atomic<bool> g_shutdown_requested{false};

/**
 * @brief Signal handler for graceful shutdown
 *
 * This function is called when the application receives termination signals
 * (SIGINT from Ctrl+C, SIGTERM from system shutdown, etc.). It sets the global
 * shutdown flag to coordinate clean termination of all threads and operations.
 *
 * @param signal The signal number that was received
 *
 * @note This function is signal-safe and only performs async-signal-safe operations
 * @see g_shutdown_requested
 */
void signalHandler(int signal) {
    std::cerr << "\nReceived signal " << signal << ". Initiating graceful shutdown..." << std::endl;
    g_shutdown_requested.store(true);
}

/**
 * @brief Main entry point for the Gaussian Extractor application
 *
 * The main function handles the complete application lifecycle:
 * 1. Sets up signal handlers for graceful shutdown
 * 2. Initializes the configuration system
 * 3. Parses command-line arguments and options
 * 4. Dispatches to the appropriate command handler
 * 5. Handles exceptions and provides appropriate exit codes
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 *
 * @return Exit code: 0 for success, non-zero for various error conditions
 *
 * @section Exit Codes
 * - 0: Successful execution
 * - 1: General error (exceptions, unknown commands, etc.)
 * - Command-specific exit codes may also be returned
 *
 * @section Signal Handling
 * The application installs handlers for SIGINT and SIGTERM to enable graceful
 * shutdown of long-running operations. When these signals are received, the
 * global shutdown flag is set and all threads are notified to terminate cleanly.
 *
 * @section Configuration
 * The configuration system is initialized first, loading settings from:
 * - Default configuration file (.gaussian_extractor.conf)
 * - User-specified configuration file
 * - Command-line overrides
 *
 * Configuration errors are reported as warnings but don't prevent execution.
 *
 * @section Error Handling
 * All exceptions are caught at the top level to ensure clean termination:
 * - std::exception derived exceptions show the error message
 * - Unknown exceptions show a generic error message
 * - All exceptions result in exit code 1
 *
 * @note This function coordinates the entire application flow and ensures
 *       proper resource cleanup even in error conditions
 */
int main(int argc, char* argv[]) {
    // Install signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Create program command dispatch table
    std::unordered_map<std::string, std::unordered_map<cck::core::CommandType, std::function<int(const cck::core::CommandContext&)>>> program_dispatch = {
        {
            "gaussian", {
                {cck::core::CommandType::EXTRACT, cck::gaussian::execute_extract_command},
                {cck::core::CommandType::CHECK_DONE, cck::gaussian::execute_check_done_command},
                {cck::core::CommandType::CHECK_ERRORS, cck::gaussian::execute_check_errors_command},
                {cck::core::CommandType::CHECK_PCM, cck::gaussian::execute_check_pcm_command},
                {cck::core::CommandType::CHECK_ALL, cck::gaussian::execute_check_all_command},
                {cck::core::CommandType::HIGH_LEVEL_KJ, cck::gaussian::execute_high_level_kj_command},
                {cck::core::CommandType::HIGH_LEVEL_AU, cck::gaussian::execute_high_level_au_command}
            }
        }
        // Future programs can be added here:
        // {"orca", { ... }},
        // {"qchem", { ... }}
    };

    try {
        // Initialize configuration system - load from file and apply defaults
        if (!g_config_manager.load_config()) {
            // Configuration loaded with warnings/errors - continue with defaults
            auto errors = g_config_manager.get_load_errors();
            if (!errors.empty()) {
                std::cerr << "Configuration warnings:" << std::endl;
                for (const auto& error : errors) {
                    std::cerr << "  " << error << std::endl;
                }
                std::cerr << std::endl;
            }
        }

        // Parse command and context (will use configuration defaults)
        cck::core::CommandContext context = cck::core::CommandParser::parse(argc, argv);

        // Show warnings if any and not in quiet mode
        if (!context.warnings.empty() && !context.quiet) {
            for (const auto& warning : context.warnings) {
                std::cerr << warning << std::endl;
            }
            std::cerr << std::endl;
        }

        // Get the default program from configuration
        std::string default_program = cck::core::get_default_program();
        
        // Show program information if not in quiet mode
        if (!context.quiet) {
            std::cout << "Using quantum chemistry program: " << default_program << std::endl;
            std::string config_path = g_config_manager.get_config_file_path();
            std::cout << "Configuration loaded from: " << (config_path.empty() ? "built-in defaults" : config_path) << std::endl;
            std::cout << std::endl;
        }
        
        // Execute based on command type and configured program
        auto program_it = program_dispatch.find(default_program);
        if (program_it == program_dispatch.end()) {
            std::cerr << "Error: Unknown or unsupported program '" << default_program << "'. Available programs: gaussian" << std::endl;
            std::cerr << "Please check your configuration file setting for 'default_program'" << std::endl;
            return 1;
        }
        
        auto command_it = program_it->second.find(context.command);
        if (command_it == program_it->second.end()) {
            std::cerr << "Error: Command not supported by program '" << default_program << "'" << std::endl;
            return 1;
        }
        
        // Execute the command using the configured program's handler
        return command_it->second(context);

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Fatal error: Unknown exception occurred" << std::endl;
        return 1;
    }

    return 0;
}
