#!/bin/bash

# AI Audio Generator - Build and Run Script
# This script builds the AI Audio Generator and runs basic tests

set -e  # Exit on any error

echo "AI Audio Generator - Build and Run Script"
echo "========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Create build directory
print_status "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
print_status "Building the project..."
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    print_success "Build completed successfully!"
else
    print_error "Build failed!"
    exit 1
fi

# Run the basic test
print_status "Running basic functionality test..."
echo ""

if [ -f "src/aiaudio_generator" ]; then
    ./src/aiaudio_generator
    if [ $? -eq 0 ]; then
        print_success "Basic test passed!"
    else
        print_error "Basic test failed!"
        exit 1
    fi
else
    print_error "Executable not found: src/aiaudio_generator"
    exit 1
fi

echo ""
print_success "AI Audio Generator is working correctly!"
print_status "You can now use the following commands:"
echo "  - Run the basic test: ./build/src/aiaudio_generator"
echo "  - Build with debug info: cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug && make"
echo "  - Clean build: cd build && make clean"
echo "  - Install dependencies: make install-deps (if you have sudo access)"