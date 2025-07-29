# CMake configuration for ComChemKit (CCK)
cmake_minimum_required(VERSION 3.16)
project(ComChemKit
    VERSION 0.1.0
    DESCRIPTION "Computational Chemistry Toolkit for Quantum Chemistry Programs"
    LANGUAGES CXX
)

# Currently only Gaussian is supported - other programs will be added in future versions
option(WITH_GAUSSIAN "Build with Gaussian support" ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

# Project options
option(ENABLE_EXTRA_WARNINGS "Enable extra compiler warnings" OFF)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(BUILD_FOR_CLUSTER "Build with cluster-specific optimizations" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Define source files
# Core CCK components
set(CCK_CORE_SOURCES
    src/core/cck_command_system.cpp
    src/core/cck_config_manager.cpp
    src/core/cck_job_scheduler.cpp
    src/core/cck_qm_program.cpp
)

set(CCK_CORE_HEADERS
    src/core/cck_command_system.h
    src/core/cck_config_manager.h
    src/core/cck_job_scheduler.h
    src/core/cck_version.h
    src/core/cck_qm_program.h
)

# Gaussian module
set(GAUSSIAN_SOURCES
    src/gaussian/gaussian_program.cpp
    src/gaussian/gaussian_extractor.cpp
    src/gaussian/gaussian_job_checker.cpp
    src/gaussian/gaussian_command_executor.cpp
    src/gaussian/gaussian_high_level_energy.cpp
)

set(GAUSSIAN_HEADERS
    src/gaussian/gaussian_program.h
    src/gaussian/gaussian_extractor.h
    src/gaussian/gaussian_job_checker.h
    src/gaussian/gaussian_high_level_energy.h
    src/gaussian/gaussian_commands.h
)

# Future modules (ORCA, NWCHEM, etc.) will be added here

# Main program
set(SOURCES
    src/main.cpp
    ${CCK_CORE_SOURCES}
)

set(HEADERS
    ${CCK_CORE_HEADERS}
)

# Add quantum chemistry program modules
if(WITH_GAUSSIAN)
    list(APPEND SOURCES ${GAUSSIAN_SOURCES})
    list(APPEND HEADERS ${GAUSSIAN_HEADERS})
    add_definitions(-DCCK_WITH_GAUSSIAN)
endif()

# Future program support will be added here
# ORCA, NWCHEM, QCHEM, etc.

# Create the executable
add_executable(cck ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(cck
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gaussian
)

# Find required packages
find_package(Threads REQUIRED)

# Compiler and platform specific settings
if(MSVC)
    target_compile_options(cck PRIVATE
        /W4
        /EHsc
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Debug>:/Od /Zi>
    )
    target_compile_definitions(cck PRIVATE
        _WIN32_WINNT=0x0601  # Windows 7 minimum
        _CRT_SECURE_NO_WARNINGS
    )
else()
    target_compile_options(cck PRIVATE
        -Wall
        -Wextra
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Debug>:-O0 -g>
    )

    if(BUILD_FOR_CLUSTER)
        target_compile_options(cck PRIVATE
            -march=x86-64-v3  # Modern CPU architecture for clusters
            -mtune=generic
        )
    endif()

    if(ENABLE_EXTRA_WARNINGS)
        target_compile_options(cck PRIVATE
            -Wpedantic
            -Wcast-align
            -Wcast-qual
            -Wconversion
            -Wdouble-promotion
            -Wformat=2
            -Winit-self
            -Wlogical-op
            -Wmissing-declarations
            -Wmissing-include-dirs
            -Wold-style-cast
            -Woverloaded-virtual
            -Wredundant-decls
            -Wshadow
            -Wsign-conversion
            -Wswitch-default
            -Wundef
        )
    endif()
endif()

# Link dependencies
target_link_libraries(cck
    PRIVATE
        Threads::Threads
)

if(WIN32)
    target_link_libraries(cck PRIVATE psapi)
endif()

# Enable Address Sanitizer if requested
if(ENABLE_ASAN AND NOT MSVC)
    target_compile_options(cck PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
    )
    target_link_options(cck PRIVATE
        -fsanitize=address
    )
endif()

# Set output name
set_target_properties(cck
    PROPERTIES
    OUTPUT_NAME "cck"
)

# Installation rules
include(GNUInstallDirs)
install(TARGETS cck
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install documentation
install(FILES
    README.MD
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# Testing configuration (if tests are added in the future)
enable_testing()
# add_subdirectory(tests)  # Uncomment when tests are added

# Print configuration summary
message(STATUS "")
message(STATUS "ComChemKit (CCK) Configuration Summary")
message(STATUS "=======================================")
message(STATUS "Version:          ${PROJECT_VERSION}")
message(STATUS "Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform:         ${CMAKE_SYSTEM_NAME}")
message(STATUS "Install prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Quantum Chemistry Programs:")
message(STATUS "  Gaussian:       ${WITH_GAUSSIAN}")
message(STATUS "  Other programs will be supported in future versions")
message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "  Extra warnings: ${ENABLE_EXTRA_WARNINGS}")
message(STATUS "  ASAN enabled:   ${ENABLE_ASAN}")
message(STATUS "  Cluster build:  ${BUILD_FOR_CLUSTER}")
message(STATUS "")
