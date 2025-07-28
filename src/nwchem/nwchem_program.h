/**
 * @file nwchem_program.h
 * @brief NWChem quantum chemistry program implementation for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 * 
 * This header implements the QMProgram interface for the NWChem quantum
 * chemistry package. It provides functionality for parsing NWChem output
 * files, extracting energies and properties, checking job status, and
 * handling NWChem-specific features.
 */

#ifndef CCK_NWCHEM_PROGRAM_H
#define CCK_NWCHEM_PROGRAM_H

#include "core/cck_qm_program.h"
#include <unordered_map>
#include <regex>

namespace cck {
namespace nwchem {

/**
 * @brief Implementation of QMProgram interface for NWChem
 * 
 * This class provides NWChem-specific implementations of all QMProgram
 * interface methods, along with additional NWChem-specific functionality.
 */
class NWChemProgram : public core::QMProgram {
public:
    /**
     * @brief Default constructor
     */
    NWChemProgram();
    
    // Implementation of QMProgram interface
    std::string get_program_name() const override { return "NWChem"; }
    bool is_valid_output_file(const std::string& filepath) const override;
    core::EnergyComponents extract_energies(const std::string& filepath) const override;
    core::CalculationMetadata get_metadata(const std::string& filepath) const override;
    core::JobStatus check_job_status(const std::string& filepath) const override;
    bool create_input_file(const std::string& filepath,
                          const std::string& method,
                          const std::vector<std::string>& keywords) const override;
    int execute_command(const core::CommandContext& context) const override;
    std::vector<std::string> get_supported_extensions() const override;
    void register_commands() override;

    // NWChem-specific methods
    
    /**
     * @brief Extract TCE coupled cluster energies
     * @param filepath Path to NWChem output file
     * @return Map of correlation energy components
     */
    std::unordered_map<std::string, double> extract_tce_energies(
        const std::string& filepath) const;
    
    /**
     * @brief Extract MCSCF/MRCI information
     * @param filepath Path to NWChem output file
     * @return Natural orbital occupations and CI coefficients
     */
    std::vector<std::pair<double, double>> extract_mcscf_info(
        const std::string& filepath) const;
    
    /**
     * @brief Extract relativistic corrections
     * @param filepath Path to NWChem output file
     * @return Map of relativistic energy corrections
     */
    std::unordered_map<std::string, double> get_relativistic_corrections(
        const std::string& filepath) const;
    
    /**
     * @brief Extract plane wave DFT information
     * @param filepath Path to NWChem output file
     * @return Band structure and DOS data if available
     */
    std::optional<std::vector<double>> get_band_structure(
        const std::string& filepath) const;

protected:
    std::string parse_output_file(const std::string& filepath) const override;
    bool validate_results(const core::EnergyComponents& energies) const override;

private:
    // Regular expressions for parsing NWChem output
    static const std::regex TOTAL_ENERGY;
    static const std::regex SCF_CONVERGENCE;
    static const std::regex FREQUENCIES;
    static const std::regex TCE_ENERGY;
    static const std::regex ERROR_MESSAGE;
    static const std::regex GEOMETRY_CONV;
    
    /**
     * @brief Extract numerical value using regex
     * @param content File content to search
     * @param pattern Regex pattern
     * @param default_value Value to return if not found
     * @return Extracted value
     */
    double extract_value(const std::string& content,
                        const std::regex& pattern,
                        double default_value = 0.0) const;
    
    /**
     * @brief Parse NWChem input directives
     * @param content File content
     * @return Map of input parameters
     */
    std::unordered_map<std::string, std::string> parse_input_directives(
        const std::string& content) const;
    
    /**
     * @brief Check for parallel execution errors
     * @param content File content
     * @return Error message if found, empty string otherwise
     */
    std::string check_parallel_errors(const std::string& content) const;
    
    /**
     * @brief Validate basis set and method compatibility
     * @param metadata Calculation metadata
     * @return true if configuration is valid
     */
    bool validate_method_basis(const core::CalculationMetadata& metadata) const;
    
    /**
     * @brief Extract memory usage information
     * @param content File content
     * @return Peak memory usage in MB
     */
    double get_peak_memory_usage(const std::string& content) const;
};

} // namespace nwchem
} // namespace cck

#endif // CCK_NWCHEM_PROGRAM_H