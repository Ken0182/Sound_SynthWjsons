# AI Audio Generator - Comprehensive Build System
# Supports both C++ audio engine and Python web interface

# Project configuration
PROJECT_NAME = aiaudio_generator
CXX = g++
PYTHON = python3
PIP = pip3
CMAKE = cmake
MAKE = make

# Directories
BUILD_DIR = build
# Optional: use CMake presets, e.g., BUILD_PRESET=msys2-mingw
BUILD_PRESET ?=
BUILD_TYPE ?= Release
SRC_DIR = src
INCLUDE_DIR = include
WEB_DIR = web
DIST_DIR = dist

# Compiler flags
CXXFLAGS = -std=c++17 -O3 -march=native -ffast-math -Wall -Wextra
DEBUG_FLAGS = -g -O0 -DDEBUG
INCLUDES = -I$(INCLUDE_DIR) -I/usr/include/opencv4
LIBS = -ljsoncpp -lyaml-cpp -lpthread -lm

# Python dependencies
PYTHON_DEPS = requirements.txt

# Web dependencies
WEB_DEPS = package.json

# Default target
.PHONY: all
all: build-cpp build-python build-web

# C++ Build targets
.PHONY: build-cpp
build-cpp:
ifeq ($(strip $(BUILD_PRESET)),)
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
	$(CMAKE) --build $(BUILD_DIR) --config $(BUILD_TYPE) -j
else
	$(CMAKE) --preset $(BUILD_PRESET)
	$(CMAKE) --build --preset $(BUILD_PRESET) --config $(BUILD_TYPE) -j
endif

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Python build and setup
.PHONY: build-python
build-python: install-python-deps

.PHONY: install-python-deps
install-python-deps:
	$(PIP) install --upgrade pip
	$(PIP) install -r $(PYTHON_DEPS)

# Web interface build
.PHONY: build-web
build-web: $(WEB_DIR)/package.json
	cd $(WEB_DIR) && npm install --no-audit --no-fund || true
	cd $(WEB_DIR) && npm run build || true

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
	cp -r $(BUILD_DIR)/src/$(PROJECT_NAME) $(DIST_DIR)/ 2>/dev/null || true
	cp -r $(WEB_DIR)/dist/* $(DIST_DIR)/ 2>/dev/null || true
	cp *.py $(DIST_DIR)/
	cp *.json $(DIST_DIR)/
	cp -r config/ $(DIST_DIR)/ 2>/dev/null || true

# Installation targets
.PHONY: install
install: dist
	@echo "Installing binary to /usr/local/bin requires privileges."
	@echo "Run manually if desired: sudo cp $(DIST_DIR)/$(PROJECT_NAME) /usr/local/bin/ && sudo chmod +x /usr/local/bin/$(PROJECT_NAME)"

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
	@echo "Install deps on Debian/Ubuntu or use MSYS2 packages on Windows."

.PHONY: install-web-deps
install-web-deps: $(WEB_DIR)/package.json
	cd $(WEB_DIR) && npm install --no-audit --no-fund || true

# Help target
.PHONY: help
help:
	@echo "AI Audio Generator - Available Targets:"
	@echo ""
	@echo "Build Targets:"
	@echo "  all          - Build everything (C++, Python, Web)"
	@echo "  build-cpp    - Build C++ (uses CMake; supports BUILD_PRESET and BUILD_TYPE)"
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
	@echo "  install-deps - Echo system dependency guidance (see docs/build/msys2_notes.md)"
	@echo "  install      - Prepare dist; prints manual install instructions"
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
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_PRESET - Use a CMake preset (e.g., msys2-mingw)"
	@echo "  BUILD_TYPE   - Release/Debug (default Release)"

# Quick start for development
.PHONY: quickstart
quickstart: install-deps build-python serve-dev
	@echo "Quick start complete! Web interface available at http://localhost:8080"