#!/bin/bash

# AI Audio Generator Build and Run Script - Fixed Version
# This script handles CMake cache issues and runs from the correct directory

set -e  # Exit on any error

echo "=== AI Audio Generator Build and Run Script (Fixed) ==="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run from the project root."
    exit 1
fi

# Clean build directory to avoid CMake cache issues
echo "Cleaning build directory..."
rm -rf build
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON

# Build the project
echo "Building project..."
make -j$(nproc)

# Check if build was successful
if [ ! -f "src/aiaudio_generator" ]; then
    echo "Error: Build failed - executable not found"
    exit 1
fi

echo "Build successful!"

# Run the main application from project root (so it can find config files)
echo "Running main application..."
cd ..
if [ -f "build/src/aiaudio_generator" ]; then
    ./build/src/aiaudio_generator
    echo "Main application completed successfully!"
else
    echo "Error: Main executable not found."
    exit 1
fi

echo "=== Build and Run Complete ==="
echo "Generated files:"
ls -la build/src/ 2>/dev/null || echo "No build artifacts found"