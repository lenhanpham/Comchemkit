/**
 * @file gaussian_program.cpp
 * @brief Implementation of Gaussian quantum chemistry program for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 * 
 * This file implements the QMProgram interface for the Gaussian quantum
 * chemistry package. It provides functionality for parsing Gaussian output
 * files, extracting energies and properties, checking job status, and
 * handling Gaussian-specific features.
 */

#include "gaussian_program.h"
#include "../core/cck_qm_program.h"
#include "../core/cck_command_system.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <cmath>

namespace cck {
namespace gaussian {

// Static regex patterns for parsing Gaussian output
const std::regex GaussianProgram::NORMAL_TERMINATION(R"(Normal termination of Gaussian)");
const std::regex GaussianProgram::SCF_ENERGY(R"(SCF Done:\s+E\([^)]+\)\s*=\s*([-\d.]+))");
const std::regex GaussianProgram::ZPE(R"(Zero-point correction=\s*([-\d.]+))");
const std::regex GaussianProgram::THERMAL_CORRECTION(R"(Thermal correction to Energy=\s*([-\d.]+))");
const std::regex GaussianProgram::FREQUENCIES(R"(Frequencies --\s*([-\d.]+)\s*([-\d.]+)?\s*([-\d.]+)?)");
const std::regex GaussianProgram::ERROR_PATTERN(R"(Error termination|Fatal Error|Erroneous write|File lengths|Error in internal coordinate system)");
const std::regex GaussianProgram::PCM_ERROR(R"(Convergence failure -- run terminated|PCM cycles did not converge|PCM optimization failed)");

GaussianProgram::GaussianProgram() {
    // Constructor implementation - register commands if needed
}

bool GaussianProgram::is_valid_output_file(const std::string& filepath) const {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    bool found_gaussian_header = false;
    int line_count = 0;
    
    // Check first 50 lines for Gaussian identifier
    while (std::getline(file, line) && line_count < 50) {
        if (line.find("Gaussian") != std::string::npos && 
            (line.find("Revision") != std::string::npos || line.find("Inc.") != std::string::npos)) {
            found_gaussian_header = true;
            break;
        }
        line_count++;
    }
    
    return found_gaussian_header;
}

core::EnergyComponents GaussianProgram::extract_energies(const std::string& filepath) const {
    core::EnergyComponents components;
    
    try {
        std::string content = parse_output_file(filepath);
        
        // Extract electronic energy (SCF energy)
        components.electronic_energy = extract_energy_value(content, SCF_ENERGY, 0.0);
        
        // Extract zero-point energy
        components.zero_point_energy = extract_energy_value(content, ZPE, 0.0);
        
        // Extract thermal correction
        components.thermal_correction = extract_energy_value(content, THERMAL_CORRECTION, 0.0);
        
        // Extract frequencies
        std::sregex_iterator freq_iter(content.begin(), content.end(), FREQUENCIES);
        std::sregex_iterator freq_end;
        
        for (; freq_iter != freq_end; ++freq_iter) {
            const std::smatch& match = *freq_iter;
            for (size_t i = 1; i < match.size(); ++i) {
                if (match[i].matched) {
                    double freq = std::stod(match[i].str());
                    components.frequencies.push_back(freq);
                    if (freq < 0) {
                        components.has_imaginary_freq = true;
                    }
                }
            }
        }
        
        // Extract other thermodynamic corrections (these would need more specific regex patterns)
        // For now, using placeholder logic - you would implement proper extraction
        
        if (!validate_results(components)) {
            throw std::runtime_error("Extracted energy components failed validation");
        }
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to extract energies from " + filepath + ": " + e.what());
    }
    
    return components;
}

core::CalculationMetadata GaussianProgram::get_metadata(const std::string& filepath) const {
    core::CalculationMetadata metadata;
    metadata.file_path = filepath;
    
    try {
        std::string content = parse_output_file(filepath);
        
        // Extract program version
        std::regex version_regex(R"(Gaussian\s+(\d+)\s+Revision\s+([A-Z]\.\d+))");
        std::smatch version_match;
        if (std::regex_search(content, version_match, version_regex)) {
            metadata.program_version = "Gaussian " + version_match[1].str() + " " + version_match[2].str();
        }
        
        // Parse route section for method and basis set
        auto route_info = parse_route_section(content);
        if (route_info.find("method") != route_info.end()) {
            metadata.method = route_info["method"];
        }
        if (route_info.find("basis") != route_info.end()) {
            metadata.basis_set = route_info["basis"];
        }
        
        // Check job status
        metadata.status = check_job_status(filepath);
        
    } catch (const std::exception& e) {
        metadata.status = core::JobStatus::ERROR;
        std::cerr << "Warning: Could not extract complete metadata from " << filepath << ": " << e.what() << std::endl;
    }
    
    return metadata;
}

core::JobStatus GaussianProgram::check_job_status(const std::string& filepath) const {
    try {
        std::string content = parse_output_file(filepath);
        
        // Check for normal termination
        if (std::regex_search(content, NORMAL_TERMINATION)) {
            return core::JobStatus::COMPLETED;
        }
        
        // Check for errors
        if (std::regex_search(content, ERROR_PATTERN)) {
            return core::JobStatus::ERROR;
        }
        
        // Check for PCM convergence issues
        if (std::regex_search(content, PCM_ERROR)) {
            return core::JobStatus::ERROR;
        }
        
        // If file exists but no termination found, assume running or interrupted
        return core::JobStatus::INTERRUPTED;
        
    } catch (const std::exception& e) {
        return core::JobStatus::UNKNOWN;
    }
}

bool GaussianProgram::create_input_file(const std::string& filepath,
                                       const std::string& method,
                                       const std::vector<std::string>& keywords) const {
    try {
        std::ofstream outfile(filepath);
        if (!outfile.is_open()) {
            return false;
        }
        
        // Write Gaussian input file header
        outfile << "%chk=" << filepath.substr(0, filepath.find_last_of('.')) << ".chk\n";
        outfile << "%mem=4GB\n";
        outfile << "%nprocshared=4\n";
        outfile << "#p " << method;
        
        for (const auto& keyword : keywords) {
            outfile << " " << keyword;
        }
        outfile << "\n\n";
        
        outfile << "Generated by ComChemKit\n\n";
        outfile << "0 1\n";  // Charge and multiplicity - would need to be parameterized
        outfile << "C 0.0 0.0 0.0\n";  // Placeholder geometry - would need to be provided
        outfile << "\n";
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error creating input file: " << e.what() << std::endl;
        return false;
    }
}

int GaussianProgram::execute_command(const core::CommandContext& context) const {
    // This would dispatch to the appropriate Gaussian command functions
    // For now, return success - the actual command execution is handled
    // by the command executor functions
    (void)context; // Suppress unused parameter warning
    return 0;
}

std::vector<std::string> GaussianProgram::get_supported_extensions() const {
    return {".log", ".out", ".LOG", ".OUT"};
}

void GaussianProgram::register_commands() {
    // Register Gaussian-specific commands with the command system
    // This would be implemented when the command registration system is ready
}

bool GaussianProgram::check_pcm_convergence(const std::string& filepath) const {
    try {
        std::string content = parse_output_file(filepath);
        return std::regex_search(content, PCM_ERROR);
    } catch (const std::exception& e) {
        return false;
    }
}

core::EnergyComponents GaussianProgram::calculate_high_level_energy(
    const std::string& low_level_path,
    const std::string& high_level_path) const {
    
    // Extract energies from both files
    core::EnergyComponents low_level = extract_energies(low_level_path);
    core::EnergyComponents high_level = extract_energies(high_level_path);
    
    // Combine: high-level electronic energy + low-level thermal corrections
    core::EnergyComponents combined = low_level;  // Start with low-level (has thermal data)
    combined.electronic_energy = high_level.electronic_energy;  // Replace with high-level electronic
    
    return combined;
}

std::vector<std::pair<double, double>> GaussianProgram::extract_frequencies(
    const std::string& filepath) const {
    
    std::vector<std::pair<double, double>> freq_intensity_pairs;
    
    try {
        std::string content = parse_output_file(filepath);
        
        // Extract frequencies and their intensities
        std::regex freq_intensity_regex(R"(Frequencies --\s*([-\d.]+)(?:\s+([-\d.]+))?(?:\s+([-\d.]+))?\s*Red\. masses --\s*([-\d.]+)(?:\s+([-\d.]+))?(?:\s+([-\d.]+))?\s*Frc consts --\s*([-\d.]+)(?:\s+([-\d.]+))?(?:\s+([-\d.]+))?\s*IR Inten --\s*([-\d.]+)(?:\s+([-\d.]+))?(?:\s+([-\d.]+))?)");
        
        std::sregex_iterator iter(content.begin(), content.end(), freq_intensity_regex);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            const std::smatch& match = *iter;
            // Extract frequency and intensity pairs
            for (int i = 1; i <= 3; ++i) {
                if (match[i].matched && match[i+9].matched) {
                    double frequency = std::stod(match[i].str());
                    double intensity = std::stod(match[i+9].str());
                    freq_intensity_pairs.emplace_back(frequency, intensity);
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Warning: Could not extract frequency data: " << e.what() << std::endl;
    }
    
    return freq_intensity_pairs;
}

std::optional<std::string> GaussianProgram::get_dispersion_type(const std::string& filepath) const {
    try {
        std::string content = parse_output_file(filepath);
        
        // Check for various dispersion corrections
        if (content.find("GD3") != std::string::npos || content.find("D3") != std::string::npos) {
            return "D3";
        }
        if (content.find("GD2") != std::string::npos || content.find("D2") != std::string::npos) {
            return "D2";
        }
        if (content.find("GD3BJ") != std::string::npos || content.find("D3BJ") != std::string::npos) {
            return "D3BJ";
        }
        
        return std::nullopt;
        
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::string GaussianProgram::parse_output_file(const std::string& filepath) const {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool GaussianProgram::validate_results(const core::EnergyComponents& energies) const {
    // Basic validation checks
    
    // Electronic energy should be negative and reasonable
    if (energies.electronic_energy > 0.0 || energies.electronic_energy < -10000.0) {
        return false;
    }
    
    // Zero-point energy should be positive if present
    if (energies.zero_point_energy < 0.0) {
        return false;
    }
    
    // Check for NaN or infinite values
    if (std::isnan(energies.electronic_energy) || std::isinf(energies.electronic_energy)) {
        return false;
    }
    
    return true;
}

double GaussianProgram::extract_energy_value(const std::string& content,
                                            const std::regex& pattern,
                                            double default_value) const {
    std::smatch match;
    if (std::regex_search(content, match, pattern)) {
        try {
            return std::stod(match[1].str());
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not parse energy value: " << match[1].str() << std::endl;
        }
    }
    return default_value;
}

std::unordered_map<std::string, std::string> GaussianProgram::parse_route_section(
    const std::string& content) const {
    
    std::unordered_map<std::string, std::string> route_info;
    
    // Find the route section (starts with # and ends with blank line)
    std::regex route_regex(R"(#[^\n]*(?:\n[^\n\r]*)*?)");
    std::smatch route_match;
    
    if (std::regex_search(content, route_match, route_regex)) {
        std::string route_section = route_match[0].str();
        
        // Parse method (very basic implementation)
        std::regex method_regex(R"((B3LYP|M06|PBE0|CAM-B3LYP|wB97XD|MP2|CCSD|G4))");
        std::smatch method_match;
        if (std::regex_search(route_section, method_match, method_regex)) {
            route_info["method"] = method_match[1].str();
        }
        
        // Parse basis set (basic implementation)
        std::regex basis_regex(R"((6-31G|6-311G|cc-pVDZ|cc-pVTZ|aug-cc-pVDZ|aug-cc-pVTZ|def2-SVP|def2-TZVP)(?:\([^)]*\))?)");
        std::smatch basis_match;
        if (std::regex_search(route_section, basis_match, basis_regex)) {
            route_info["basis"] = basis_match[1].str();
        }
    }
    
    return route_info;
}

std::string GaussianProgram::check_error_type(const std::string& content) const {
    // Check for specific error patterns
    if (content.find("Error termination") != std::string::npos) {
        return "Error termination";
    }
    if (content.find("Convergence failure") != std::string::npos) {
        return "Convergence failure";
    }
    if (content.find("File lengths do not match") != std::string::npos) {
        return "File length mismatch";
    }
    if (content.find("Fatal Error") != std::string::npos) {
        return "Fatal error";
    }
    
    return "";
}

bool GaussianProgram::validate_calculation_type(const core::CalculationMetadata& metadata) const {
    // Validate that the calculation type is suitable for energy extraction
    
    // Check if it's a valid quantum chemistry method
    if (metadata.method.empty()) {
        return false;
    }
    
    // Basic validation - could be expanded
    return true;
}

} // namespace gaussian
} // namespace cck