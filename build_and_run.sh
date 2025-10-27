#!/bin/bash

# AI Audio Generator Build and Run Script

set -e  # Exit on any error

echo "=== AI Audio Generator Build and Run Script ==="

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run from the project root."
    exit 1
fi

if [ -f "CMakePresets.json" ]; then
    echo "Using CMake presets (ninja-release)"
    cmake --preset ninja-release
    cmake --build --preset ninja-release
    ctest --preset ninja-release --output-on-failure || true
    BINARY_DIR="build/ninja-release"
else
    echo "No CMakePresets.json found; falling back to Makefiles in ./build"
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build . -- -j$(nproc)
    ctest --output-on-failure || true
    BINARY_DIR="."
fi

# Run tests
echo "Running tests..."
if [ -f "$BINARY_DIR/tests/aiaudio_tests" ]; then
    "$BINARY_DIR/tests/aiaudio_tests"
    echo "Tests completed successfully!"
else
    echo "Warning: Test executable not found. Skipping tests."
fi

# Run the main application
echo "Running main application..."
if [ -f "$BINARY_DIR/aiaudio_generator" ]; then
    "$BINARY_DIR/aiaudio_generator"
    echo "Main application completed successfully!"
else
    echo "Error: Main executable not found at $BINARY_DIR/aiaudio_generator"
    exit 1
fi

# Run example usage
echo "Running example usage..."
if [ -f "example_usage" ]; then
    ./example_usage
    echo "Example usage completed successfully!"
else
    echo "Warning: Example executable not found. Skipping example."
fi

echo "=== Build and Run Complete ==="
echo "Generated files:"
ls -la "$BINARY_DIR"/*.raw 2>/dev/null || echo "No audio files generated"
echo "Build artifacts in $BINARY_DIR:"
ls -la "$BINARY_DIR"