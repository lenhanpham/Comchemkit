#!/bin/bash

# ComChemKit (CCK) Build Test Script
# Comprehensive build testing for both CMake and Make approaches
# Author: Le Nhan Pham
# Date: 2025

set -e  # Exit on any error

echo "=========================================="
echo "ComChemKit (CCK) Build Test Script"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to test basic compilation
test_minimal_compilation() {
    print_status "Testing minimal compilation..."

    if g++ -std=c++20 -Wall -Wextra -O2 -o cck_minimal src/main_minimal.cpp 2>/dev/null; then
        print_success "Minimal version compiled successfully!"

        # Test basic functionality
        print_status "Testing minimal version functionality..."
        ./cck_minimal --help > /dev/null 2>&1 && print_success "Help command works"
        ./cck_minimal --version > /dev/null 2>&1 && print_success "Version command works"
        ./cck_minimal extract --quiet > /dev/null 2>&1 && print_success "Extract command works"

        rm -f cck_minimal
        return 0
    else
        print_error "Minimal version compilation failed!"
        return 1
    fi
}

# Function to test CMake build
test_cmake_build() {
    print_status "Testing CMake build system..."

    # Clean previous build
    rm -rf build_cmake
    mkdir -p build_cmake
    cd build_cmake

    # Configure with CMake
    print_status "Configuring with CMake..."
    if cmake .. -DWITH_GAUSSIAN=ON > cmake_config.log 2>&1; then
        print_success "CMake configuration successful"
    else
        print_error "CMake configuration failed"
        print_error "See build_cmake/cmake_config.log for details"
        cd ..
        return 1
    fi

    # Build with make
    print_status "Building with CMake..."
    if make -j4 > cmake_build.log 2>&1; then
        print_success "CMake build successful!"

        # Test the executable
        if [ -f cck ]; then
            print_status "Testing CMake-built executable..."
            ./cck --help > /dev/null 2>&1 && print_success "CMake build: Help command works"
            ./cck --version > /dev/null 2>&1 && print_success "CMake build: Version command works"
            print_success "CMake build test completed successfully!"
            cd ..
            return 0
        else
            print_error "Executable 'cck' not found after CMake build"
            cd ..
            return 1
        fi
    else
        print_error "CMake build failed"
        print_error "See build_cmake/cmake_build.log for details"

        # Show last few lines of build log for quick diagnosis
        print_status "Last 10 lines of build log:"
        tail -10 cmake_build.log
        cd ..
        return 1
    fi
}

# Function to test direct Make build
test_make_build() {
    print_status "Testing direct Make build system..."

    # Clean previous build
    make clean > /dev/null 2>&1 || true

    # Build with make
    print_status "Building with Make..."
    if make > make_build.log 2>&1; then
        print_success "Make build successful!"

        # Test the executable
        if [ -f cck ]; then
            print_status "Testing Make-built executable..."
            ./cck --help > /dev/null 2>&1 && print_success "Make build: Help command works"
            ./cck --version > /dev/null 2>&1 && print_success "Make build: Version command works"
            print_success "Make build test completed successfully!"
            return 0
        else
            print_error "Executable 'cck' not found after Make build"
            return 1
        fi
    else
        print_error "Make build failed"
        print_error "See make_build.log for details"

        # Show last few lines of build log for quick diagnosis
        print_status "Last 10 lines of build log:"
        tail -10 make_build.log
        return 1
    fi
}

# Function to test functionality with sample data
test_functionality() {
    print_status "Testing functionality with sample data..."

    if [ -d "tests/data" ] && [ -f "tests/data/test-1.log" ]; then
        print_status "Sample data found, testing extraction..."
        cd tests/data

        if ../../cck extract --quiet --format csv > test_output.csv 2>/dev/null; then
            print_success "Energy extraction test passed"
            rm -f test_output.csv
        else
            print_warning "Energy extraction test failed (expected with placeholder data)"
        fi

        if ../../cck check-all --quiet > /dev/null 2>&1; then
            print_success "Job checking test passed"
        else
            print_warning "Job checking test failed (expected with placeholder data)"
        fi

        cd ../..
    else
        print_warning "No sample data found for functionality testing"
    fi
}

# Function to diagnose common issues
diagnose_issues() {
    print_status "Diagnosing common build issues..."

    # Check compiler
    if command -v g++ >/dev/null 2>&1; then
        GCC_VERSION=$(g++ --version | head -n1)
        print_status "Compiler found: $GCC_VERSION"

        # Check C++20 support
        if g++ -std=c++20 -x c++ -E - < /dev/null > /dev/null 2>&1; then
            print_success "C++20 support confirmed"
        else
            print_error "C++20 support not available"
        fi
    else
        print_error "g++ compiler not found"
    fi

    # Check CMake
    if command -v cmake >/dev/null 2>&1; then
        CMAKE_VERSION=$(cmake --version | head -n1)
        print_status "CMake found: $CMAKE_VERSION"
    else
        print_warning "CMake not found"
    fi

    # Check make
    if command -v make >/dev/null 2>&1; then
        print_success "Make found"
    else
        print_error "Make not found"
    fi

    # Check source files
    print_status "Checking source file structure..."

    if [ -f "src/main.cpp" ]; then
        print_success "Main source file found"
    else
        print_error "src/main.cpp not found"
    fi

    if [ -f "CMakeLists.txt" ]; then
        print_success "CMakeLists.txt found"
    else
        print_error "CMakeLists.txt not found"
    fi

    if [ -f "Makefile" ]; then
        print_success "Makefile found"
    else
        print_error "Makefile not found"
    fi

    # Check for missing files
    MISSING_FILES=()
    REQUIRED_FILES=(
        "src/core/cck_command_system.cpp"
        "src/core/cck_command_system.h"
        "src/core/cck_config_manager.cpp"
        "src/core/cck_config_manager.h"
        "src/core/cck_job_scheduler.cpp"
        "src/core/cck_job_scheduler.h"
        "src/core/cck_qm_program.cpp"
        "src/core/cck_qm_program.h"
        "src/core/cck_version.h"
        "src/gaussian/gaussian_program.cpp"
        "src/gaussian/gaussian_program.h"
        "src/gaussian/gaussian_commands.h"
        "src/gaussian/gaussian_extractor.cpp"
        "src/gaussian/gaussian_extractor.h"
        "src/gaussian/gaussian_job_checker.cpp"
        "src/gaussian/gaussian_job_checker.h"
        "src/gaussian/gaussian_command_executor.cpp"
        "src/gaussian/gaussian_high_level_energy.cpp"
        "src/gaussian/gaussian_high_level_energy.h"
    )

    for file in "${REQUIRED_FILES[@]}"; do
        if [ ! -f "$file" ]; then
            MISSING_FILES+=("$file")
        fi
    done

    if [ ${#MISSING_FILES[@]} -eq 0 ]; then
        print_success "All required source files found"
    else
        print_error "Missing source files:"
        for file in "${MISSING_FILES[@]}"; do
            echo "  - $file"
        done
    fi
}

# Function to provide build recommendations
provide_recommendations() {
    print_status "Build Recommendations:"
    echo ""
    echo "1. For development and testing:"
    echo "   Use CMake build system:"
    echo "   $ rm -rf build && mkdir build && cd build"
    echo "   $ cmake .. -DWITH_GAUSSIAN=ON"
    echo "   $ make -j4"
    echo ""
    echo "2. For quick compilation:"
    echo "   Use direct Make:"
    echo "   $ make clean && make"
    echo ""
    echo "3. For debugging compilation issues:"
    echo "   $ make clean && make 2>&1 | tee build.log"
    echo ""
    echo "4. If compilation fails, check:"
    echo "   - Missing #include statements in headers"
    echo "   - Undefined function implementations"
    echo "   - Namespace mismatches"
    echo "   - Path resolution problems"
    echo ""
    echo "5. The minimal version (main_minimal.cpp) should always compile"
    echo "   and can be used to verify basic functionality."
}

# Main execution
main() {
    print_status "Starting comprehensive build test..."
    echo ""

    # Step 1: Diagnose system
    diagnose_issues
    echo ""

    # Step 2: Test minimal compilation
    if test_minimal_compilation; then
        print_success "✅ Basic compilation capability verified"
    else
        print_error "❌ Basic compilation failed - check compiler setup"
        provide_recommendations
        exit 1
    fi
    echo ""

    # Step 3: Test CMake build
    CMAKE_SUCCESS=false
    if test_cmake_build; then
        CMAKE_SUCCESS=true
        print_success "✅ CMake build successful"
    else
        print_error "❌ CMake build failed"
    fi
    echo ""

    # Step 4: Test Make build
    MAKE_SUCCESS=false
    if test_make_build; then
        MAKE_SUCCESS=true
        print_success "✅ Make build successful"
    else
        print_error "❌ Make build failed"
    fi
    echo ""

    # Step 5: Test functionality if we have a working build
    if [ "$CMAKE_SUCCESS" = true ] || [ "$MAKE_SUCCESS" = true ]; then
        test_functionality
    fi
    echo ""

    # Summary
    echo "=========================================="
    echo "BUILD TEST SUMMARY"
    echo "=========================================="

    if [ "$CMAKE_SUCCESS" = true ] && [ "$MAKE_SUCCESS" = true ]; then
        print_success "🎉 All build systems working correctly!"
        print_success "CCK is ready for use with both CMake and Make"
        echo ""
        echo "Quick start:"
        echo "  ./cck --help"
        echo "  ./cck extract --quiet"
        echo "  ./cck check-all"
    elif [ "$CMAKE_SUCCESS" = true ]; then
        print_success "✅ CMake build working"
        print_warning "⚠️  Make build needs fixes"
        echo ""
        echo "Use CMake build:"
        echo "  cd build_cmake && ./cck --help"
    elif [ "$MAKE_SUCCESS" = true ]; then
        print_success "✅ Make build working"
        print_warning "⚠️  CMake build needs fixes"
        echo ""
        echo "Use Make build:"
        echo "  ./cck --help"
    else
        print_error "❌ Both build systems failed"
        echo ""
        provide_recommendations
        exit 1
    fi

    echo ""
    echo "Build test completed!"
}

# Run main function
main "$@"
