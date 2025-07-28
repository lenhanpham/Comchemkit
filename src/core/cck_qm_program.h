/**
 * @file cck_qm_program.h
 * @brief Abstract interface for quantum chemistry program modules in ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 * 
 * This header defines the abstract interface that all quantum chemistry program
 * modules must implement to be integrated with ComChemKit. It provides a common
 * set of operations and data structures for handling different quantum chemistry
 * packages (Gaussian, ORCA, NWCHEM, etc.).
 */

#ifndef CCK_QM_PROGRAM_H
#define CCK_QM_PROGRAM_H

#include "cck_version.h"
#include "cck_command_system.h"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <functional>
#include <unordered_map>

namespace cck {
namespace core {

/**
 * @brief Structure to hold energy components from quantum chemistry calculations
 */
struct EnergyComponents {
    double electronic_energy = 0.0;      ///< Total electronic energy
    double zero_point_energy = 0.0;      ///< Zero-point vibrational energy
    double thermal_correction = 0.0;      ///< Thermal correction to energy
    double enthalpy_correction = 0.0;     ///< Correction to enthalpy
    double gibbs_correction = 0.0;        ///< Correction to Gibbs free energy
    double entropy = 0.0;                 ///< Total entropy
    double nuclear_repulsion = 0.0;       ///< Nuclear repulsion energy
    std::vector<double> frequencies;      ///< Vibrational frequencies
    bool has_imaginary_freq = false;      ///< Whether system has imaginary frequencies
    
    // Optional components that may be program-specific
    std::optional<double> dispersion_correction;  ///< Empirical dispersion correction
    std::optional<double> solvation_energy;       ///< Solvation free energy
    std::optional<double> counterpoise_correction; ///< BSSE correction
};

/**
 * @brief Job status enumeration
 */
enum class JobStatus {
    UNKNOWN,        ///< Status cannot be determined
    COMPLETED,      ///< Job completed successfully
    ERROR,          ///< Job terminated with error
    RUNNING,        ///< Job is still running
    INTERRUPTED     ///< Job was interrupted
};

/**
 * @brief Structure to hold calculation metadata
 */
struct CalculationMetadata {
    std::string program_version;          ///< Version of QM program used
    std::string method;                   ///< Calculation method/level of theory
    std::string basis_set;                ///< Basis set used
    std::vector<std::string> keywords;    ///< Additional calculation keywords
    std::optional<std::string> solvent;   ///< Solvent model if used
    double temperature = 298.15;          ///< Temperature in Kelvin
    double pressure = 1.0;                ///< Pressure in atm
    std::string file_path;                ///< Path to output file
    JobStatus status = JobStatus::UNKNOWN; ///< Current job status
};

/**
 * @brief Abstract base class for quantum chemistry program modules
 * 
 * This class defines the interface that must be implemented by all quantum
 * chemistry program modules in ComChemKit. It provides methods for reading
 * output files, extracting energies and properties, checking job status,
 * and handling program-specific features.
 */
class QMProgram {
public:
    virtual ~QMProgram() = default;
    
    /**
     * @brief Get the name of the quantum chemistry program
     * @return Program name (e.g., "Gaussian", "ORCA", etc.)
     */
    virtual std::string get_program_name() const = 0;
    
    /**
     * @brief Check if a file is a valid output file for this program
     * @param filepath Path to the file to check
     * @return true if file is a valid output file
     */
    virtual bool is_valid_output_file(const std::string& filepath) const = 0;
    
    /**
     * @brief Extract energy components from an output file
     * @param filepath Path to output file
     * @return EnergyComponents structure with extracted values
     * @throw std::runtime_error if extraction fails
     */
    virtual EnergyComponents extract_energies(const std::string& filepath) const = 0;
    
    /**
     * @brief Get metadata about the calculation
     * @param filepath Path to output file
     * @return CalculationMetadata structure with calculation details
     */
    virtual CalculationMetadata get_metadata(const std::string& filepath) const = 0;
    
    /**
     * @brief Check the status of a job
     * @param filepath Path to output file
     * @return JobStatus indicating current status
     */
    virtual JobStatus check_job_status(const std::string& filepath) const = 0;
    
    /**
     * @brief Create input file for a calculation
     * @param filepath Path where input file should be written
     * @param method Calculation method/level of theory
     * @param keywords Additional calculation keywords
     * @return true if input file was created successfully
     */
    virtual bool create_input_file(const std::string& filepath,
                                 const std::string& method,
                                 const std::vector<std::string>& keywords) const = 0;
    
    /**
     * @brief Execute program-specific commands
     * @param context Command context containing parameters
     * @return Exit code (0 for success)
     */
    virtual int execute_command(const CommandContext& context) const = 0;
    
    /**
     * @brief Get supported file extensions
     * @return Vector of supported file extensions (e.g., {".log", ".out"})
     */
    virtual std::vector<std::string> get_supported_extensions() const = 0;
    
    /**
     * @brief Register program-specific commands
     * 
     * This method should be called during program initialization to register
     * any program-specific commands with the command system.
     */
    virtual void register_commands() = 0;
    
protected:
    /**
     * @brief Parse output file content
     * @param filepath Path to output file
     * @return Parsed content ready for extraction
     */
    virtual std::string parse_output_file(const std::string& filepath) const = 0;
    
    /**
     * @brief Validate calculation results
     * @param energies Extracted energy components
     * @return true if results are physically reasonable
     */
    virtual bool validate_results(const EnergyComponents& energies) const = 0;
};

/**
 * @brief Create an instance of a specific QM program module
 * @param program_name Name of the program ("gaussian", "orca", etc.)
 * @return Unique pointer to QMProgram instance
 * @throw std::runtime_error if program is not supported
 */
std::unique_ptr<QMProgram> create_qm_program(const std::string& program_name);

/**
 * @brief Register all available QM program modules
 * 
 * This function should be called during program initialization to register
 * all available quantum chemistry program modules with the system.
 */
void register_qm_programs();

} // namespace core
} // namespace cck

#endif // CCK_QM_PROGRAM_H