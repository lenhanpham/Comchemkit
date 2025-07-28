/**
 * @file gaussian_program.h
 * @brief Gaussian quantum chemistry program implementation for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 *
 * This header implements the QMProgram interface for the Gaussian quantum
 * chemistry package. It provides functionality for parsing Gaussian output
 * files, extracting energies and properties, checking job status, and
 * handling Gaussian-specific features.
 */

#ifndef CCK_GAUSSIAN_PROGRAM_H
#define CCK_GAUSSIAN_PROGRAM_H

#include "../core/cck_qm_program.h"
#include <unordered_map>
#include <regex>

namespace cck {
namespace gaussian {

/**
 * @brief Implementation of QMProgram interface for Gaussian
 *
 * This class provides Gaussian-specific implementations of all QMProgram
 * interface methods, along with additional Gaussian-specific functionality.
 */
class GaussianProgram : public core::QMProgram {
public:
    /**
     * @brief Default constructor
     */
    GaussianProgram();

    // Implementation of QMProgram interface
    std::string get_program_name() const override { return "Gaussian"; }
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

    // Gaussian-specific methods

    /**
     * @brief Check for PCM convergence issues
     * @param filepath Path to Gaussian output file
     * @return true if PCM convergence issues were found
     */
    bool check_pcm_convergence(const std::string& filepath) const;

    /**
     * @brief Calculate high-level energy corrections
     * @param low_level_path Path to low-level calculation output
     * @param high_level_path Path to high-level single point calculation
     * @return Combined energy components with high-level corrections
     */
    core::EnergyComponents calculate_high_level_energy(
        const std::string& low_level_path,
        const std::string& high_level_path) const;

    /**
     * @brief Extract frequency information
     * @param filepath Path to Gaussian output file
     * @return Vector of frequencies and their intensities
     */
    std::vector<std::pair<double, double>> extract_frequencies(
        const std::string& filepath) const;

    /**
     * @brief Check if calculation includes empirical dispersion
     * @param filepath Path to Gaussian output file
     * @return Type of dispersion correction if found
     */
    std::optional<std::string> get_dispersion_type(const std::string& filepath) const;

protected:
    std::string parse_output_file(const std::string& filepath) const override;
    bool validate_results(const core::EnergyComponents& energies) const override;

private:
    // Regular expressions for parsing Gaussian output
    static const std::regex NORMAL_TERMINATION;
    static const std::regex SCF_ENERGY;
    static const std::regex ZPE;
    static const std::regex THERMAL_CORRECTION;
    static const std::regex FREQUENCIES;
    static const std::regex ERROR_PATTERN;
    static const std::regex PCM_ERROR;

    /**
     * @brief Extract a specific energy value using regex
     * @param content File content to search
     * @param pattern Regex pattern for the energy
     * @param default_value Value to return if not found
     * @return Extracted energy value
     */
    double extract_energy_value(const std::string& content,
                              const std::regex& pattern,
                              double default_value = 0.0) const;

    /**
     * @brief Parse Gaussian route section
     * @param content File content
     * @return Map of keywords and their values
     */
    std::unordered_map<std::string, std::string> parse_route_section(
        const std::string& content) const;

    /**
     * @brief Check for specific error types
     * @param content File content
     * @return Error message if found, empty string otherwise
     */
    std::string check_error_type(const std::string& content) const;

    /**
     * @brief Validate Gaussian calculation type
     * @param metadata Calculation metadata
     * @return true if calculation type is valid for energy extraction
     */
    bool validate_calculation_type(const core::CalculationMetadata& metadata) const;
};

} // namespace gaussian
} // namespace cck

#endif // CCK_GAUSSIAN_PROGRAM_H
