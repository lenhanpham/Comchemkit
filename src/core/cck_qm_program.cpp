/**
 * @file cck_qm_program.cpp
 * @brief Implementation of QMProgram registry and factory functions
 * @author Le Nhan Pham
 * @date 2025
 */

#include "cck_qm_program.h"
#include "../gaussian/gaussian_program.h"
#include <stdexcept>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <functional>

namespace cck {
namespace core {

namespace {
    // Registry of program factories
    std::unordered_map<std::string, std::function<std::unique_ptr<QMProgram>()>> program_registry;
    
    // Helper function to convert program name to lowercase for case-insensitive comparison
    std::string normalize_program_name(const std::string& name) {
        std::string normalized = name;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return normalized;
    }
}

std::unique_ptr<QMProgram> create_qm_program(const std::string& program_name) {
    auto normalized_name = normalize_program_name(program_name);
    auto it = program_registry.find(normalized_name);
    
    if (it == program_registry.end()) {
        throw std::runtime_error("Unsupported quantum chemistry program: " + program_name);
    }
    
    return it->second();
}

void register_qm_programs() {
    // Register Gaussian program (currently the only implemented program)
    #ifdef CCK_WITH_GAUSSIAN
    program_registry["gaussian"] = []() {
        return std::make_unique<gaussian::GaussianProgram>();
    };
    #endif

    // Placeholder registrations for future programs
    #ifdef CCK_WITH_ORCA
    // Will be implemented in future versions
    program_registry["orca"] = []() {
        throw std::runtime_error("ORCA support is not yet implemented");
        return nullptr;
    };
    #endif

    #ifdef CCK_WITH_NWCHEM
    // Will be implemented in future versions
    program_registry["nwchem"] = []() {
        throw std::runtime_error("NWChem support is not yet implemented");
        return nullptr;
    };
    #endif

    #ifdef CCK_WITH_QCHEM
    // Will be implemented in future versions
    program_registry["qchem"] = []() {
        throw std::runtime_error("Q-Chem support is not yet implemented");
        return nullptr;
    };
    #endif
}

std::vector<std::string> get_supported_programs() {
    std::vector<std::string> supported;
    supported.reserve(program_registry.size());
    
    for (const auto& entry : program_registry) {
        supported.push_back(entry.first);
    }
    
    return supported;
}

bool is_program_supported(const std::string& program_name) {
    return program_registry.find(normalize_program_name(program_name)) != program_registry.end();
}

void register_qm_program(const std::string& name, 
                        std::function<std::unique_ptr<QMProgram>()> factory) {
    program_registry[normalize_program_name(name)] = std::move(factory);
}

} // namespace core
} // namespace cck