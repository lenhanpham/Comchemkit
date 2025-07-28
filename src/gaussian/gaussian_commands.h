/**
 * @file gaussian_commands.h
 * @brief Command function declarations for Gaussian module in ComChemKit
 * @author Le Nhan Pham
 * @date 2025
 *
 * This header declares all command execution functions for the Gaussian
 * quantum chemistry module. These functions handle the various operations
 * that can be performed on Gaussian output files, including energy
 * extraction, job status checking, and high-level energy calculations.
 */

#ifndef CCK_GAUSSIAN_COMMANDS_H
#define CCK_GAUSSIAN_COMMANDS_H

#include "../core/cck_command_system.h"

namespace cck {
namespace gaussian {

/**
 * @brief Execute energy extraction command for Gaussian log files
 * @param context Command context containing extraction parameters
 * @return Exit code (0 for success, non-zero for error)
 *
 * Extracts thermodynamic data from Gaussian log files in the current directory.
 * Supports multi-threaded processing and various output formats.
 */
int execute_extract_command(const cck::core::CommandContext& context);

/**
 * @brief Execute job completion check command
 * @param context Command context containing check parameters
 * @return Exit code (0 for success, non-zero for error)
 *
 * Checks Gaussian log files to identify which jobs have completed successfully.
 * Reports files with normal termination status.
 */
int execute_check_done_command(const cck::core::CommandContext& context);

/**
 * @brief Execute error checking command
 * @param context Command context containing check parameters
 * @return Exit code (0 for success, non-zero for error)
 *
 * Scans Gaussian log files for error conditions and failure modes.
 * Reports files with error termination or other issues.
 */
int execute_check_errors_command(const cck::core::CommandContext& context);

/**
 * @brief Execute PCM convergence check command
 * @param context Command context containing check parameters
 * @return Exit code (0 for success, non-zero for error)
 *
 * Specifically checks for PCM (Polarizable Continuum Model) convergence
 * issues in Gaussian calculations using implicit solvation.
 */
int execute_check_pcm_command(const cck::core::CommandContext& context);

/**
 * @brief Execute comprehensive check command
 * @param context Command context containing check parameters
 * @return Exit code (0 for success, non-zero for error)
 *
 * Performs all available checks (completion, errors, PCM) on Gaussian
 * log files and provides a comprehensive status report.
 */
int execute_check_all_command(const cck::core::CommandContext& context);

/**
 * @brief Execute high-level energy calculation (kJ/mol output)
 * @param context Command context containing calculation parameters
 * @return Exit code (0 for success, non-zero for error)
 *
 * Combines high-level single-point electronic energies with low-level
 * thermal corrections to compute accurate thermodynamic properties.
 * Results are output in kJ/mol units.
 */
int execute_high_level_kj_command(const cck::core::CommandContext& context);

/**
 * @brief Execute high-level energy calculation (atomic units output)
 * @param context Command context containing calculation parameters
 * @return Exit code (0 for success, non-zero for error)
 *
 * Combines high-level single-point electronic energies with low-level
 * thermal corrections to compute accurate thermodynamic properties.
 * Results are output in atomic units (Hartree).
 */
int execute_high_level_au_command(const cck::core::CommandContext& context);

} // namespace gaussian
} // namespace cck

#endif // CCK_GAUSSIAN_COMMANDS_H
