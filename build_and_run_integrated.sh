#!/bin/bash
set -e

echo "=== AI Audio Generator - Integrated Build Script ==="
echo "Building Python-C++ integrated system..."

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build/
rm -rf *.so
rm -rf aiaudio_python*.so

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DPYTHON_EXECUTABLE=$(which python3) \
    -DCMAKE_PREFIX_PATH=$(python3 -c "import pybind11; print(pybind11.get_cmake_dir())")

# Build the project
echo "Building C++ library..."
make -j$(nproc)

# Build Python module
echo "Building Python module..."
cd ..
python3 setup.py build_ext --inplace

# Install Python dependencies
echo "Installing Python dependencies..."
pip3 install -r requirements.txt

# Test the build
echo "Testing the build..."
python3 -c "
try:
    import aiaudio_python
    print('✓ C++ module imported successfully')
except ImportError as e:
    print(f'✗ Failed to import C++ module: {e}')

try:
    from cpp_engine import create_hybrid_interface
    engine = create_hybrid_interface()
    print('✓ Hybrid interface created successfully')
except Exception as e:
    print(f'✗ Failed to create hybrid interface: {e}')
"

# Run a test generation
echo "Running test generation..."
python3 main.py --query "warm pad" --role "PAD" --generate-audio --output test_output.wav --duration 1.0

if [ -f "test_output.wav" ]; then
    echo "✓ Test audio generated successfully"
    ls -la test_output.wav
else
    echo "✗ Test audio generation failed"
fi

echo ""
echo "=== Build Complete ==="
echo "To run the web server:"
echo "  python3 web_server.py"
echo ""
echo "To generate audio:"
echo "  python3 main.py --query 'your prompt' --generate-audio"
echo ""
echo "To test C++ engine:"
echo "  python3 cpp_engine.py"