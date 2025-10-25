#!/bin/bash

# AI Audio Generator Build and Run Script

set -e  # Exit on any error

echo "=== AI Audio Generator Build and Run Script ==="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run from the project root."
    exit 1
fi

echo "Preparing clean out-of-source build..."
rm -rf build

# Configure with CMake (out-of-source)
echo "Configuring with CMake..."
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON

# Build the project
echo "Building project..."
cmake --build build -- -j$(nproc)

# Run tests
echo "Running tests..."
if [ -f "build/tests/aiaudio_tests" ]; then
    ./build/tests/aiaudio_tests
    echo "Tests completed successfully!"
else
    echo "Warning: Test executable not found. Skipping tests."
fi

# Run the main application
echo "Running main application..."
if [ -f "build/src/aiaudio_generator" ]; then
    ./build/src/aiaudio_generator
    echo "Main application completed successfully!"
else
    echo "Error: Main executable not found."
    exit 1
fi

# Run example usage
echo "Running example usage..."
if [ -f "build/example_usage" ]; then
    ./build/example_usage
    echo "Example usage completed successfully!"
else
    echo "Warning: Example executable not found. Skipping example."
fi

echo "=== Build and Run Complete ==="
echo "Generated files:"
ls -la *.raw 2>/dev/null || echo "No audio files generated"
echo "Build artifacts:"
ls -la build