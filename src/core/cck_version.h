/**
 * @file cck_version.h
 * @brief Version information and build metadata for ComChemKit (CCK)
 * @author Le Nhan Pham
 * @date 2025
 *
 * This header file contains all version-related constants, macros, and utility
 * functions for the ComChemKit (CCK) application. It provides centralized
 * version management and build information that can be used throughout the
 * application for display, logging, and compatibility checking.
 *
 * @section Version Management
 * Version information is defined using semantic versioning (MAJOR.MINOR.PATCH):
 * - MAJOR: Incremented for incompatible API changes
 * - MINOR: Incremented for backward-compatible functionality additions
 * - PATCH: Incremented for backward-compatible bug fixes
 *
 * @section Build Information
 * Additional build metadata includes:
 * - Application name and description
 * - Copyright and author information
 * - Repository URL for source code
 *
 * @note When updating version numbers, ensure CMakeLists.txt PROJECT(VERSION)
 *       is also updated to maintain consistency across build systems
 */

#ifndef CCK_VERSION_H
#define CCK_VERSION_H

#include <string>
#include <cstring>

/**
 * @defgroup VersionConstants Version Constants
 * @brief Preprocessor constants defining version information
 * @{
 */

/**
 * @brief Major version number
 *
 * Incremented when incompatible API changes are made.
 * Breaking changes that require user intervention.
 */
#define CCK_VERSION_MAJOR 1

/**
 * @brief Minor version number
 *
 * Incremented when backward-compatible functionality is added.
 * New features that don't break existing usage.
 */
#define CCK_VERSION_MINOR 0

/**
 * @brief Patch version number
 *
 * Incremented for backward-compatible bug fixes.
 * No new features, only corrections to existing functionality.
 */
#define CCK_VERSION_PATCH 0

/**
 * @brief Complete version string
 *
 * Human-readable version string combining all version components.
 * Format: "vMAJOR.MINOR.PATCH"
 */
#define CCK_VERSION_STRING "v0.1.0"

/** @} */ // end of VersionConstants group

/**
 * @defgroup BuildInfo Build Information
 * @brief Constants containing application metadata
 * @{
 */

/**
 * @brief Application name
 */
#define CCK_NAME "ComChemKit"

/**
 * @brief Brief description of the application's purpose
 */
#define CCK_DESCRIPTION "Multi-program computational chemistry toolkit for quantum chemistry calculations"

/** @} */ // end of BuildInfo group

/**
 * @defgroup AuthorInfo Author and Copyright Information
 * @brief Constants containing authorship and legal information
 * @{
 */

/**
 * @brief Copyright notice
 */
#define CCK_COPYRIGHT "Copyright (c) 2025 Le Nhan Pham"

/**
 * @brief Primary author
 */
#define CCK_AUTHOR "Le Nhan Pham"

/**
 * @brief Source code repository URL
 */
#define CCK_REPOSITORY "https://github.com/lenhanpham/cck"

/** @} */ // end of AuthorInfo group

/**
 * @namespace GaussianExtractor
 * @brief Main namespace containing version utilities and functions
 */
namespace cck {
namespace core {
    /**
     * @defgroup VersionFunctions Version Utility Functions
     * @brief Functions for retrieving and manipulating version information
     * @{
     */

    /**
     * @brief Get the version string
     * @return Version string in format "vMAJOR.MINOR.PATCH"
     *
     * Returns just the version number without application name or other metadata.
     * Useful for version comparisons and logging where only the version is needed.
     */
    inline std::string get_version() {
        return std::string(CCK_VERSION_STRING);
    }

    /**
     * @brief Get the full version with application name
     * @return Application name followed by version string
     *
     * Returns a combination of the application name and version suitable for
     * display in headers, about dialogs, or command-line version output.
     */
    inline std::string get_full_version() {
        return std::string(CCK_NAME) + " " +
               std::string(CCK_VERSION_STRING);
    }

    /**
     * @brief Get comprehensive version information
     * @return Multi-line string with complete version and build information
     *
     * Returns a formatted multi-line string containing:
     * - Application name and version
     * - Description
     * - Copyright notice
     * - Repository URL
     *
     * Suitable for detailed version displays or help output.
     */
    inline std::string get_version_info() {
        return std::string(CCK_NAME) + " " +
               std::string(CCK_VERSION_STRING) + "\n" +
               std::string(CCK_DESCRIPTION) + "\n" +
               std::string(CCK_COPYRIGHT) + "\n" +
               std::string(CCK_REPOSITORY);
    }

    /**
     * @brief Get header information for program output
     * @return Single-line header with name, version, and author
     *
     * Returns a concise single-line string suitable for program headers
     * or banner displays. Contains application name, version, and author.
     */
    inline std::string get_header_info() {
        return std::string(CCK_NAME) + " " +
               std::string(CCK_VERSION_STRING) + " developed by " +
               std::string(CCK_AUTHOR);
    }

    /**
     * @brief Check if current version meets minimum requirements
     * @param major Minimum required major version
     * @param minor Minimum required minor version
     * @param patch Minimum required patch version (default: 0)
     * @return true if current version >= required version, false otherwise
     *
     * Performs semantic version comparison to determine if the current
     * application version meets or exceeds the specified minimum version.
     * Useful for compatibility checks and feature availability testing.
     *
     * @note Version comparison follows semantic versioning rules:
     *       - Major version takes precedence
     *       - Minor version compared only if major versions equal
     *       - Patch version compared only if major and minor versions equal
     */
    inline bool is_version_at_least(int major, int minor, int patch = 0) {
        if (CCK_VERSION_MAJOR > major) return true;
        if (CCK_VERSION_MAJOR < major) return false;
        if (CCK_VERSION_MINOR > minor) return true;
        if (CCK_VERSION_MINOR < minor) return false;
        return CCK_VERSION_PATCH >= patch;
    }

    /** @} */ // end of VersionFunctions group
} // namespace core
} // namespace cck

#endif // CCK_VERSION_H
