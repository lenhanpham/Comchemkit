/**
 * @file orca_program.h
 * @brief ORCA quantum chemistry program implementation for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 * 
 * This header implements the QMProgram interface for the ORCA quantum
 * chemistry package. It provides functionality for parsing ORCA output
 * files, extracting energies and properties, checking job status, and
 * handling ORCA-specific features.
 */

#ifndef CCK_ORCA_PROGRAM_H
#define CCK_ORCA_PROGRAM_H

#include "core/cck_qm_program.h"
#include <unordered_map>
#include <regex>

namespace cck {
namespace orca {

/**
 * @brief Implementation of QMProgram interface for ORCA
 * 
 * This class provides ORCA-specific implementations of all QMProgram
 * interface methods, along with additional ORCA-specific functionality.
 */
class OrcaProgram : public core::QMProgram {
public:
    /**
     * @brief Default constructor
     */
    OrcaProgram();
    
    // Implementation of QMProgram interface
    std::string get_program_name() const override { return "ORCA"; }
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

    // ORCA-specific methods
    
    /**
     * @brief Extract DLPNO-CCSD(T) correlation energies
     * @param filepath Path to ORCA output file
     * @return Map of correlation energy components
     */
    std::unordered_map<std::string, double> extract_dlpno_energies(
        const std::string& filepath) const;
    
    /**
     * @brief Extract CASSCF/NEVPT2 information
     * @param filepath Path to ORCA output file
     * @return Natural orbital occupations and energies
     */
    std::vector<std::pair<double, double>> extract_casscf_info(
        const std::string& filepath) const;
    
    /**
     * @brief Check for ORCA-specific convergence issues
     * @param filepath Path to ORCA output file
     * @return true if convergence issues were found
     */
    bool check_convergence_issues(const std::string& filepath) const;

protected:
    std::string parse_output_file(const std::string& filepath) const override;
    bool validate_results(const core::EnergyComponents& energies) const override;

private:
    // Regular expressions for parsing ORCA output
    static const std::regex FINAL_ENERGY;
    static const std::regex SCF_CONVERGENCE;
    static const std::regex FREQUENCIES;
    static const std::regex ERROR_MESSAGE;
    
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
     * @brief Parse ORCA input block
     * @param content File content
     * @return Map of input parameters
     */
    std::unordered_map<std::string, std::string> parse_input_block(
        const std::string& content) const;
    
    /**
     * @brief Validate calculation setup
     * @param metadata Calculation metadata
     * @return true if calculation setup is valid
     */
    bool validate_calculation_setup(const core::CalculationMetadata& metadata) const;
};

} // namespace orca
} // namespace cck

#endif // CCK_ORCA_PROGRAM_H