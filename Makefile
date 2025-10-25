# AI Audio Generator - Simplified Makefile
# Delegates to CMake for cross-platform builds

.PHONY: all clean install test help configure build python

# Default target
all: configure build

# Configuration
configure:
	@echo "Configuring build with CMake..."
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
build: configure
	@echo "Building project..."
	@cd build && make -j$(shell nproc 2>/dev/null || echo 4)

# Python module
python: configure
	@echo "Building Python module..."
	@cd build && make aiaudio_python -j$(shell nproc 2>/dev/null || echo 4)
	@echo "Installing Python module..."
	@pip3 install -e .

# Web application
web: python
	@echo "Building React web application..."
	@./build_web.sh

# Install
install: build
	@echo "Installing..."
	@cd build && make install

# Test
test: build
	@echo "Running tests..."
	@cd build && ctest --output-on-failure

# Clean
clean:
	@echo "Cleaning build directories..."
	@rm -rf build build-* *.so *.pyd aiaudio_python*.so
	@rm -rf dist build *.egg-info
	@find . -name "*.pyc" -delete
	@find . -name "__pycache__" -delete

# Clean all (including dependencies)
clean-all: clean
	@echo "Cleaning all generated files..."
	@rm -rf .venv venv
	@rm -rf test_output.wav

# MSYS2 specific targets
msys2-mingw:
	@echo "Configuring for MSYS2 MinGW..."
	@mkdir -p build-msys2
	@cd build-msys2 && cmake .. -G "MinGW Makefiles" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_PREFIX_PATH="C:/msys64/mingw64" \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_CXX_COMPILER=g++ \
		-DCMAKE_RC_COMPILER=windres
	@cd build-msys2 && make -j$(shell nproc 2>/dev/null || echo 4)

msys2-ucrt:
	@echo "Configuring for MSYS2 UCRT..."
	@mkdir -p build-msys2-ucrt
	@cd build-msys2-ucrt && cmake .. -G "MinGW Makefiles" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_PREFIX_PATH="C:/msys64/ucrt64" \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_CXX_COMPILER=g++ \
		-DCMAKE_RC_COMPILER=windres
	@cd build-msys2-ucrt && make -j$(shell nproc 2>/dev/null || echo 4)

# Debug build
debug:
	@echo "Configuring debug build..."
	@mkdir -p build-debug
	@cd build-debug && cmake .. -DCMAKE_BUILD_TYPE=Debug
	@cd build-debug && make -j$(shell nproc 2>/dev/null || echo 4)

# Release build
release:
	@echo "Configuring release build..."
	@mkdir -p build-release
	@cd build-release && cmake .. -DCMAKE_BUILD_TYPE=Release
	@cd build-release && make -j$(shell nproc 2>/dev/null || echo 4)

# Development setup
dev-setup:
	@echo "Setting up development environment..."
	@pip3 install -r requirements.txt
	@make python
	@make web

# Run web server
server: python
	@echo "Starting web server..."
	@python3 web_server.py

# Run audio generation test
test-audio: python
	@echo "Testing audio generation..."
	@python3 main.py --query "warm pad" --role "PAD" --generate-audio --output test_output.wav --duration 1.0

# Run C++ engine test
test-cpp: python
	@echo "Testing C++ engine..."
	@python3 cpp_engine.py

# Full integration test
test-integration: python
	@echo "Running integration tests..."
	@python3 main.py --query "aggressive bass" --role "BASS" --generate-audio --output test_bass.wav
	@python3 main.py --query "bright lead" --role "LEAD" --generate-audio --output test_lead.wav
	@python3 main.py --query "atmospheric pad" --role "PAD" --generate-audio --output test_pad.wav
	@echo "Integration test complete. Check test_*.wav files."

# Help
help:
	@echo "AI Audio Generator - Available targets:"
	@echo "  all              - Configure and build (default)"
	@echo "  configure        - Configure with CMake"
	@echo "  build            - Build the project"
	@echo "  python           - Build and install Python module"
	@echo "  install          - Install the project"
	@echo "  test             - Run tests"
	@echo "  clean            - Clean build directories"
	@echo "  clean-all        - Clean everything including dependencies"
	@echo "  msys2-mingw      - Build for MSYS2 MinGW"
	@echo "  msys2-ucrt       - Build for MSYS2 UCRT"
	@echo "  debug            - Debug build"
	@echo "  release          - Release build"
	@echo "  dev-setup        - Set up development environment"
	@echo "  server           - Start web server"
	@echo "  test-audio       - Test audio generation"
	@echo "  test-cpp         - Test C++ engine"
	@echo "  test-integration - Run full integration test"
	@echo "  help             - Show this help"