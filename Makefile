# AI Audio Generator - Cross-Platform Build System
# Supports Linux, macOS, and MSYS2/Windows

# Detect operating system
ifeq ($(OS),Windows_NT)
    # Windows/MSYS2 detection
    ifeq ($(shell uname -o 2>/dev/null),Msys)
        PLATFORM := msys2
        CMAKE_PRESET := msys2-mingw64
        PYTHON := python
        MAKE_PROGRAM := mingw32-make
        NPROC := $(shell nproc)
    else
        PLATFORM := windows
        CMAKE_PRESET := default
        PYTHON := python
        MAKE_PROGRAM := make
        NPROC := 4
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM := linux
        CMAKE_PRESET := linux-gcc
        PYTHON := python3
        MAKE_PROGRAM := make
        NPROC := $(shell nproc)
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macos
        CMAKE_PRESET := macos
        PYTHON := python3
        MAKE_PROGRAM := make
        NPROC := $(shell sysctl -n hw.ncpu)
    endif
endif

# Project configuration
PROJECT_NAME = aiaudio_generator
CXX = g++
CMAKE = cmake

# Directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
WEB_DIR = web
DIST_DIR = dist

# Compiler flags
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
ifeq ($(PLATFORM),msys2)
    # MSYS2 specific flags
    LIBS = -ljsoncpp -lyaml-cpp
else
    # Linux/macOS flags
    CXXFLAGS += -march=native -ffast-math
    LIBS = -ljsoncpp -lyaml-cpp -lpthread -lm
endif

DEBUG_FLAGS = -g -O0 -DDEBUG
INCLUDES = -I$(INCLUDE_DIR)

# Python dependencies
PYTHON_DEPS = requirements.txt

# Web dependencies
WEB_DEPS = package.json

# Default target
.PHONY: all
all: build-cpp build-python build-web

# C++ Build targets
.PHONY: build-cpp
build-cpp: $(BUILD_DIR)/$(PROJECT_NAME)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(PROJECT_NAME): $(BUILD_DIR)
ifeq ($(PLATFORM),msys2)
	@echo "Building for MSYS2/MinGW64"
	cd $(BUILD_DIR) && $(CMAKE) -G "MinGW Makefiles" .. -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON && $(MAKE_PROGRAM)
else
	@echo "Building for $(PLATFORM)"
	cd $(BUILD_DIR) && $(CMAKE) .. -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON && $(MAKE_PROGRAM) -j$(NPROC)
endif

# Python build and setup
.PHONY: build-python
build-python: install-python-deps

.PHONY: install-python-deps
install-python-deps:
	$(PYTHON) -m pip install --upgrade pip
	$(PYTHON) -m pip install -r $(PYTHON_DEPS)

# Web interface build
.PHONY: build-web
build-web: $(WEB_DIR)/package.json
	cd $(WEB_DIR) && npm install
	cd $(WEB_DIR) && npm run build

$(WEB_DIR)/package.json: | $(WEB_DIR)
	@echo "Creating web package.json..."

$(WEB_DIR):
	mkdir -p $(WEB_DIR)

# Development targets
.PHONY: dev
dev: build-python
	$(PYTHON) web_server.py

.PHONY: dev-cpp
dev-cpp: build-cpp
	./$(BUILD_DIR)/src/$(PROJECT_NAME)

# Testing targets
.PHONY: test
test: test-python test-cpp

.PHONY: test-python
test-python: build-python
	$(PYTHON) -m pytest tests/ -v

.PHONY: test-cpp
test-cpp: build-cpp
	cd $(BUILD_DIR) && ctest --verbose

# Audio system specific targets
.PHONY: audio-demo
audio-demo: build-python
	$(PYTHON) audio_interface.py --demo

.PHONY: load-presets
load-presets: build-python
	$(PYTHON) audio_interface.py --load-presets

# Web server targets
.PHONY: serve
serve: build-web build-python
	$(PYTHON) web_server.py --host 0.0.0.0 --port 8080

.PHONY: serve-dev
serve-dev: build-python
	$(PYTHON) web_server.py --debug --host 0.0.0.0 --port 8080

# Distribution targets
.PHONY: dist
dist: clean all
	mkdir -p $(DIST_DIR)
	cp -r $(BUILD_DIR)/$(PROJECT_NAME) $(DIST_DIR)/
	cp -r $(WEB_DIR)/dist/* $(DIST_DIR)/ 2>/dev/null || true
	cp *.py $(DIST_DIR)/
	cp *.json $(DIST_DIR)/
	cp -r config/ $(DIST_DIR)/ 2>/dev/null || true

# Installation targets
.PHONY: install
install: dist
	sudo cp $(DIST_DIR)/$(PROJECT_NAME) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(PROJECT_NAME)

.PHONY: install-web
install-web: build-web
	sudo cp -r $(WEB_DIR)/dist/* /var/www/html/ 2>/dev/null || true

# Clean targets
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DIST_DIR)
	rm -rf $(WEB_DIR)/node_modules
	rm -rf $(WEB_DIR)/dist
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -delete

.PHONY: clean-cpp
clean-cpp:
	rm -rf $(BUILD_DIR)

.PHONY: clean-python
clean-python:
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -delete

.PHONY: clean-web
clean-web:
	rm -rf $(WEB_DIR)/node_modules
	rm -rf $(WEB_DIR)/dist

# Dependencies installation
.PHONY: install-deps
install-deps: install-system-deps install-python-deps install-web-deps

.PHONY: install-system-deps
install-system-deps:
ifeq ($(PLATFORM),msys2)
	@echo "Installing dependencies for MSYS2..."
	pacman -S --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-yaml-cpp mingw-w64-x86_64-jsoncpp mingw-w64-x86_64-python mingw-w64-x86_64-pybind11
else ifeq ($(PLATFORM),macos)
	@echo "Installing dependencies for macOS..."
	brew install cmake yaml-cpp jsoncpp python pybind11 node
else
	@echo "Installing dependencies for Linux..."
	sudo apt-get update
	sudo apt-get install -y cmake g++ python3 python3-pip nodejs npm
	sudo apt-get install -y libyaml-cpp-dev libjsoncpp-dev pybind11-dev
	sudo apt-get install -y libgtest-dev libgmock-dev pkg-config
endif

.PHONY: install-web-deps
install-web-deps: $(WEB_DIR)/package.json
	cd $(WEB_DIR) && npm install

# Help target
.PHONY: help
help:
	@echo "AI Audio Generator - Cross-Platform Build System"
	@echo "================================================"
	@echo ""
	@echo "Detected platform: $(PLATFORM)"
	@echo "CMake preset: $(CMAKE_PRESET)"
	@echo "Python: $(PYTHON)"
	@echo ""
	@echo "Build Targets:"
	@echo "  all          - Build everything (C++, Python, Web)"
	@echo "  build-cpp    - Build C++ audio engine"
	@echo "  build-python - Install Python dependencies"
	@echo "  build-web    - Build web interface"
	@echo ""
	@echo "Development:"
	@echo "  dev          - Start Python development server"
	@echo "  dev-cpp      - Run C++ application"
	@echo "  serve        - Start production web server"
	@echo "  serve-dev    - Start development web server"
	@echo ""
	@echo "Audio System:"
	@echo "  audio-demo   - Run audio demonstration"
	@echo "  load-presets - Load and display available presets"
	@echo ""
	@echo "Testing:"
	@echo "  test         - Run all tests"
	@echo "  test-python  - Run Python tests"
	@echo "  test-cpp     - Run C++ tests"
	@echo ""
	@echo "Installation:"
	@echo "  install-deps - Install all system dependencies"
	@echo "  install      - Install application"
	@echo "  install-web  - Install web interface"
	@echo ""
	@echo "Cleanup:"
	@echo "  clean        - Clean all build artifacts"
	@echo "  clean-cpp    - Clean C++ build"
	@echo "  clean-python - Clean Python artifacts"
	@echo "  clean-web    - Clean web build"
	@echo ""
	@echo "Distribution:"
	@echo "  dist         - Create distribution package"

# Quick start for development
.PHONY: quickstart
quickstart: install-deps build-python serve-dev
	@echo "Quick start complete! Web interface available at http://localhost:8080"