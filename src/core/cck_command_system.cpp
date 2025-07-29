/**
 * @file cck_command_system.cpp
 * @brief Implementation of command line parsing and execution system for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 */

#include "cck_command_system.h"
#include "cck_config_manager.h"
#include "cck_version.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace cck {
namespace core {

std::string get_default_program() {
    try {
        // Try to get the default program from configuration
        return g_config_manager.get_string("default_program", "gaussian");
    } catch (...) {
        // If configuration is not loaded or any error occurs, return default
        return "gaussian";
    }
}

CommandContext CommandParser::parse(int argc, char* argv[]) {
    // Early check for version before any other processing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--version" || arg == "-v") {
            std::cout << "ComChemKit (CCK) version " << CCK_VERSION_STRING << std::endl;
            std::exit(0);
        }
        if (arg == "--help" || arg == "-h") {
            show_help();
            std::exit(0);
        }
    }

    CommandContext context;
    
    // Apply default values
    context.command = CommandType::EXTRACT;
    context.temperature = 298.15;
    context.concentration = 1.0;
    context.thread_count = -1;
    context.sort_column = 2;
    context.format = "text";
    context.extension = ".log";
    context.quiet = false;
    context.max_file_size = 100;
    context.memory_limit = 0;

    // If no arguments, default to EXTRACT
    if (argc == 1) {
        return context;
    }

    // Check if first argument is a command
    std::string first_arg = argv[1];
    CommandType potential_command = parse_command(first_arg);

    int start_index = 1;
    if (potential_command != CommandType::EXTRACT || first_arg == "extract") {
        context.command = potential_command;
        start_index = 2; // Skip the command argument
    }

    // Parse remaining arguments
    for (int i = start_index; i < argc; ++i) {
        std::string arg = argv[i];

        // Common options
        if (arg == "-q" || arg == "--quiet") {
            context.quiet = true;
        }
        else if (arg == "-e" || arg == "--ext") {
            if (++i < argc) {
                std::string ext = argv[i];
                context.extension = (ext[0] == '.') ? ext : ("." + ext);
            } else {
                context.warnings.push_back("Error: Extension required after --ext");
            }
        }
        else if (arg == "-j" || arg == "--threads") {
            if (++i < argc) {
                try {
                    int threads = std::stoi(argv[i]);
                    if (threads > 0) {
                        context.thread_count = threads;
                    } else {
                        context.warnings.push_back("Error: Thread count must be positive. Using auto-detection.");
                        context.thread_count = -1;
                    }
                } catch (const std::exception& e) {
                    context.warnings.push_back("Error: Invalid thread count. Using auto-detection.");
                    context.thread_count = -1;
                }
            } else {
                context.warnings.push_back("Error: Thread count required after --threads");
            }
        }
        else if (arg == "--max-size") {
            if (++i < argc) {
                try {
                    size_t size = std::stoull(argv[i]);
                    context.max_file_size = size;
                } catch (const std::exception& e) {
                    context.warnings.push_back("Error: Invalid file size. Using default.");
                }
            } else {
                context.warnings.push_back("Error: File size required after --max-size");
            }
        }
        else if (arg == "--memory") {
            if (++i < argc) {
                try {
                    size_t mem = std::stoull(argv[i]);
                    context.memory_limit = mem;
                } catch (const std::exception& e) {
                    context.warnings.push_back("Error: Invalid memory limit. Using auto-detection.");
                }
            } else {
                context.warnings.push_back("Error: Memory limit required after --memory");
            }
        }
        else if (arg == "--format") {
            if (++i < argc) {
                std::string fmt = argv[i];
                if (fmt == "text" || fmt == "csv" || fmt == "json") {
                    context.format = fmt;
                } else {
                    context.warnings.push_back("Error: Format must be 'text', 'csv', or 'json'. Using default 'text'.");
                    context.format = "text";
                }
            } else {
                context.warnings.push_back("Error: Format required after --format");
            }
        }
        
        // Extract-specific options
        else if (context.command == CommandType::EXTRACT) {
            if (arg == "-t" || arg == "--temp") {
                if (++i < argc) {
                    try {
                        double temp = std::stod(argv[i]);
                        if (temp > 0) {
                            context.temperature = temp;
                        } else {
                            context.warnings.push_back("Warning: Temperature must be positive. Using default 298.15 K.");
                            context.temperature = 298.15;
                        }
                    } catch (const std::exception& e) {
                        context.warnings.push_back("Error: Invalid temperature format. Using default 298.15 K.");
                        context.temperature = 298.15;
                    }
                } else {
                    context.warnings.push_back("Error: Temperature required after --temp");
                }
            }
            else if (arg == "-c" || arg == "--conc") {
                if (++i < argc) {
                    try {
                        double conc = std::stod(argv[i]);
                        if (conc > 0) {
                            context.concentration = conc;
                        } else {
                            context.warnings.push_back("Error: Concentration must be positive. Using default 1.0.");
                            context.concentration = 1.0;
                        }
                    } catch (const std::exception& e) {
                        context.warnings.push_back("Error: Invalid concentration format. Using default 1.0.");
                        context.concentration = 1.0;
                    }
                } else {
                    context.warnings.push_back("Error: Concentration required after --conc");
                }
            }
            else if (arg == "--sort") {
                if (++i < argc) {
                    try {
                        int col = std::stoi(argv[i]);
                        if (col >= 1 && col <= 10) {
                            context.sort_column = col;
                        } else {
                            context.warnings.push_back("Error: Sort column must be between 1 and 10. Using default 2.");
                            context.sort_column = 2;
                        }
                    } catch (const std::exception& e) {
                        context.warnings.push_back("Error: Invalid sort column. Using default 2.");
                        context.sort_column = 2;
                    }
                } else {
                    context.warnings.push_back("Error: Sort column required after --sort");
                }
            }
        }
        
        // Unknown argument
        else {
            context.warnings.push_back("Warning: Unknown argument '" + arg + "' ignored.");
        }
    }

    return context;
}

CommandType CommandParser::parse_command(const std::string& cmd) {
    if (cmd == "extract") return CommandType::EXTRACT;
    if (cmd == "check-done") return CommandType::CHECK_DONE;
    if (cmd == "check-errors") return CommandType::CHECK_ERRORS;
    if (cmd == "check-pcm") return CommandType::CHECK_PCM;
    if (cmd == "check-all") return CommandType::CHECK_ALL;
    if (cmd == "high-level-kj") return CommandType::HIGH_LEVEL_KJ;
    if (cmd == "high-level-au") return CommandType::HIGH_LEVEL_AU;

    // If it starts with '-', it's probably an option, not a command
    if (!cmd.empty() && cmd.front() == '-') return CommandType::EXTRACT;
    
    return CommandType::EXTRACT; // Default to extract for unknown commands
}

void CommandParser::show_help(const std::string& command) {
    if (command.empty()) {
        std::cout << "ComChemKit (CCK) - Computational Chemistry Toolkit\n\n";
        std::cout << "Usage: cck <command> [options]\n\n";
        std::cout << "Commands:\n";
        std::cout << "  extract         Extract thermodynamic data from Gaussian log files (default)\n";
        std::cout << "  check-done      Check for completed Gaussian calculations\n";
        std::cout << "  check-errors    Check for Gaussian calculation errors\n";
        std::cout << "  check-pcm       Check for PCM convergence issues\n";
        std::cout << "  check-all       Run all status checks\n";
        std::cout << "  high-level-kj   High-level energy calculations (kJ/mol)\n";
        std::cout << "  high-level-au   High-level energy calculations (Hartree)\n\n";
        std::cout << "Global Options:\n";
        std::cout << "  -q, --quiet       Suppress non-essential output\n";
        std::cout << "  -e, --ext <EXT>   File extension to process (default: log)\n";
        std::cout << "  -j, --threads <N> Number of threads (-1 = auto-detect)\n";
        std::cout << "  --max-size <MB>   Maximum file size in MB (default: 100)\n";
        std::cout << "  --memory <MB>     Memory limit in MB (0 = auto)\n";
        std::cout << "  --format <FMT>    Output format: text|csv|json (default: text)\n";
        std::cout << "  -h, --help        Show this help message\n";
        std::cout << "  --version         Show version information\n\n";
        std::cout << "Configuration:\n";
        std::cout << "  Config file locations (searched in order):\n";
        std::cout << "    ./.cck.conf (current directory)\n";
        std::cout << "    ~/.cck.conf (home directory)\n";
        std::cout << "  Use 'default_program = <program>' to set quantum chemistry program\n";
        std::cout << "  Available programs: gaussian (more coming soon)\n\n";
        std::cout << "Extract Options:\n";
        std::cout << "  -t, --temp <T>    Temperature in Kelvin (default: 298.15)\n";
        std::cout << "  -c, --conc <C>    Concentration in mol/L (default: 1.0)\n";
        std::cout << "  --sort <COL>      Column to sort by (1-10, default: 2)\n\n";
        std::cout << "Examples:\n";
        std::cout << "  cck extract --temp 298.15 --conc 1.0 --format csv\n";
        std::cout << "  cck check-all --quiet\n";
        std::cout << "  cck high-level-kj --temp 298.15\n";
    } else {
        std::cout << "Help for specific command: " << command << std::endl;
        // Command-specific help would go here
    }
}

} // namespace core
} // namespace cck