#!/bin/bash

# Find Constants Script for ComChemKit
# This script searches source files for potential hardcoded constants
# that should be moved to the universal constants system.

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_finding() {
    echo -e "${BLUE}[FOUND]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "Please run this script from the root directory of the ComChemKit project"
    exit 1
fi

print_info "Scanning for potential hardcoded constants..."

# Search patterns
patterns=(
    # Physical constants
    '[0-9]+\.[0-9]+e[-+][0-9]+' # Scientific notation
    'const.*double.*='  # Constant declarations
    'constexpr.*double.*='  # Constexpr declarations
    '#define.*[0-9]' # Numeric defines

    # Common physical values
    '298\.15'  # Room temperature
    '273\.15'  # 0°C in Kelvin
    '101325'   # Standard pressure
    '1\.0'     # Unity values that might be constants

    # Unit conversion factors
    'hartree'  # Case-insensitive
    'kcal'
    'kj.*mol'
    'ev'

    # Other common constants
    'pi'
    'avogadro'
    'boltzmann'
    'planck'
)

# File patterns to search
file_patterns=(
    '*.cpp'
    '*.h'
    '*.hpp'
)

print_info "Searching in source files..."

# Create a temporary file for results
temp_file=$(mktemp)

# Search for each pattern
for pattern in "${patterns[@]}"; do
    print_info "Searching for pattern: $pattern"

    for file_pattern in "${file_patterns[@]}"; do
        find src -type f -name "$file_pattern" -exec grep -Hn -i "$pattern" {} \; >> "$temp_file"
    done
done

# Sort and remove duplicates
sort -u "$temp_file" > "$temp_file.sorted"

# Print findings in organized sections
print_info "\nFindings by category:"

echo -e "\n${BLUE}Physical Constants:${NC}"
grep -i "boltzmann\|planck\|avogadro\|pi" "$temp_file.sorted"

echo -e "\n${BLUE}Temperature and Pressure Constants:${NC}"
grep -i "298\.15\|273\.15\|101325" "$temp_file.sorted"

echo -e "\n${BLUE}Unit Conversion Factors:${NC}"
grep -i "hartree\|kcal\|kj.*mol\|ev" "$temp_file.sorted"

echo -e "\n${BLUE}Other Numeric Constants:${NC}"
grep -v -i "boltzmann\|planck\|avogadro\|pi\|298\.15\|273\.15\|101325\|hartree\|kcal\|kj.*mol\|ev" "$temp_file.sorted"

# Clean up
rm "$temp_file" "$temp_file.sorted"

print_info "\nSuggested actions:"
print_info "1. Review each constant and determine if it should be moved to cck_constants.h"
print_info "2. Check for duplicate values that might represent the same constant"
print_info "3. Ensure consistent units are used across the codebase"
print_info "4. Update documentation for any moved constants"

print_info "\nScript completed!"
