/**
 * @file cck_constants.h
 * @brief Universal constants and default values for ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 * 
 * This header defines universal constants, physical constants, and default values
 * that can be used across all quantum chemistry program modules. It provides
 * centralized management of common values and ensures consistency across the
 * toolkit.
 */

#ifndef CCK_CONSTANTS_H
#define CCK_CONSTANTS_H

#include <string>
#include <limits>
#include <vector>

namespace cck {
namespace constants {

// Physical constants (in atomic units unless specified)
namespace physical {
    constexpr double BOLTZMANN = 3.166811563e-6;     // Boltzmann constant in Hartree/K
    constexpr double GAS_CONSTANT = 8.314462618;     // Gas constant in J/(mol·K)
    constexpr double AVOGADRO = 6.02214076e23;      // Avogadro's number
    constexpr double PLANCK = 4.135667696e-15;       // Planck constant in eV·s
    constexpr double SPEED_OF_LIGHT = 137.036;       // Speed of light in atomic units
    constexpr double HARTREE_TO_EV = 27.211386245;   // Conversion: Hartree to eV
    constexpr double HARTREE_TO_KCAL = 627.509474;   // Conversion: Hartree to kcal/mol
    constexpr double HARTREE_TO_KJ = 2625.5002;    // Conversion: Hartree to kJ/mol
    constexpr double BOHR_TO_ANGSTROM = 0.529177249; // Conversion: Bohr to Ångström
    constexpr double Po = 101325;                     // Standard pressure in Pa
    constexpr double PHASE_CORR_FACTOR = 0.0003808798033989866; // Phase correction factor
}

// Default values for calculations
namespace defaults {
    constexpr double TEMPERATURE = 298.15;           // Default temperature in Kelvin
    constexpr double PRESSURE = 1.0;                 // Default pressure in atm
    constexpr double CONCENTRATION = 1.0;            // Default concentration in mol/L
    constexpr double PHASE_CORRECTION = 1.89;        // Gas (1 atm) → Solution (1M) in kcal/mol
    constexpr double MIN_FREQ_THRESHOLD = -50.0;     // Minimum acceptable frequency in cm⁻¹
    constexpr double ZERO_THRESHOLD = 1e-10;         // Threshold for considering a value as zero
    constexpr int DECIMAL_PRECISION = 6;             // Default decimal precision for output
}

// System resource limits and thresholds
namespace limits {
    constexpr size_t MAX_FILE_SIZE_MB = 100;        // Maximum file size to process in MB
    constexpr size_t MIN_MEMORY_MB = 512;           // Minimum required memory in MB
    constexpr size_t DEFAULT_MEMORY_LIMIT = 0;       // 0 means auto-calculate
    constexpr int MAX_THREADS = 32;                  // Maximum number of threads
    constexpr int MAX_FILE_HANDLES = 20;             // Maximum concurrent file handles
    constexpr size_t MAX_LINE_LENGTH = 1024;        // Maximum line length for parsing
}

// File extensions and patterns
namespace files {
    const std::vector<std::string> INPUT_EXTENSIONS = {".com", ".gjf", ".gau"};
    const std::vector<std::string> OUTPUT_EXTENSIONS = {".log", ".out"};
    const std::string CONFIG_FILENAME = ".cck.conf";
    const std::string BACKUP_EXTENSION = ".bak";
    const std::string RESULTS_SUFFIX = ".results";
}

// Error and warning thresholds
namespace thresholds {
    constexpr double ENERGY_CONVERGENCE = 1e-6;      // Energy convergence threshold in Hartree
    constexpr double GEOMETRY_CONVERGENCE = 1e-4;    // Geometry convergence threshold
    constexpr double MAX_FORCE = 0.01;              // Maximum acceptable force in optimization
    constexpr double MAX_DISPLACEMENT = 0.05;       // Maximum acceptable displacement
    constexpr int MAX_SCF_CYCLES = 128;            // Maximum SCF cycles before considering failure
}

// Directory names and patterns
namespace directories {
    const std::string ERROR_DIR = "errorJobs";
    const std::string DONE_DIR = "done";
    const std::string PCM_ERROR_DIR = "PCMMkU";
    const std::string BACKUP_DIR = "backup";
}

// Output formatting
namespace format {
    constexpr char CSV_SEPARATOR = ',';
    constexpr char FIELD_SEPARATOR = '\t';
    constexpr bool USE_SCIENTIFIC = false;          // Use scientific notation by default
    const std::string DATE_FORMAT = "%Y-%m-%d %H:%M:%S";
    const std::string NULL_VALUE = "N/A";
    const std::vector<int> DEFAULT_COLUMN_WIDTHS = {20, 12, 10, 12, 12, 12, 10, 8, 5, 5};
}

// Common status and error messages
namespace status {
    const std::string COMPLETED = "DONE";
    const std::string INCOMPLETE = "UNDONE";
    const std::string ERROR = "ERROR";
    const std::string RUNNING = "RUNNING";
    const std::string UNKNOWN = "UNKNOWN";
}

} // namespace constants
} // namespace cck

#endif // CCK_CONSTANTS_H