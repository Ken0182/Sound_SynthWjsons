# Build Instructions - AI Synthesizer

This document provides detailed build instructions for the AI Synthesizer project across different platforms.

## Prerequisites

### Required Software

- **CMake 3.20+** - Build system generator
- **C++17 Compiler** - GCC 7+, Clang 5+, or MSVC 2017+
- **yaml-cpp** - YAML configuration library

### Platform-Specific Requirements

#### Windows (MSYS2 MinGW32)
- MSYS2 with MinGW32 toolchain
- yaml-cpp development package

#### Linux
- GCC or Clang compiler
- yaml-cpp development package
- Make or Ninja build system

#### macOS
- Xcode Command Line Tools
- yaml-cpp (via Homebrew or MacPorts)

## Installation Steps

### Windows (MSYS2 MinGW32)

1. **Install MSYS2**
   ```bash
   # Download and install MSYS2 from https://www.msys2.org/
   # Follow the installation instructions
   ```

2. **Update MSYS2**
   ```bash
   pacman -Syu
   pacman -Su
   ```

3. **Install MinGW32 toolchain**
   ```bash
   pacman -S mingw-w64-i686-gcc
   pacman -S mingw-w64-i686-cmake
   pacman -S mingw-w64-i686-make
   ```

4. **Install yaml-cpp**
   ```bash
   pacman -S mingw-w64-i686-yaml-cpp
   ```

5. **Verify installation**
   ```bash
   gcc --version
   cmake --version
   ```

### Linux (Ubuntu/Debian)

1. **Update package list**
   ```bash
   sudo apt update
   ```

2. **Install build tools**
   ```bash
   sudo apt install build-essential cmake
   ```

3. **Install yaml-cpp**
   ```bash
   sudo apt install libyaml-cpp-dev
   ```

4. **Verify installation**
   ```bash
   gcc --version
   cmake --version
   ```

### Linux (CentOS/RHEL/Fedora)

1. **Install build tools**
   ```bash
   # CentOS/RHEL
   sudo yum groupinstall "Development Tools"
   sudo yum install cmake3
   
   # Fedora
   sudo dnf groupinstall "Development Tools"
   sudo dnf install cmake
   ```

2. **Install yaml-cpp**
   ```bash
   # CentOS/RHEL
   sudo yum install yaml-cpp-devel
   
   # Fedora
   sudo dnf install yaml-cpp-devel
   ```

3. **Verify installation**
   ```bash
   gcc --version
   cmake --version
   ```

### macOS

1. **Install Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

2. **Install Homebrew** (if not already installed)
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. **Install dependencies**
   ```bash
   brew install cmake yaml-cpp
   ```

4. **Verify installation**
   ```bash
   clang --version
   cmake --version
   ```

## Building the Project

### Using CMake Presets (Recommended)

The project includes several CMake presets for different platforms and configurations:

#### Windows (MSYS2 MinGW32)
```bash
# Release build
cmake --preset windows-msys2-mingw32
cmake --build --preset windows-msys2-mingw32

# Debug build
cmake --preset windows-msys2-mingw32-debug
cmake --build --preset windows-msys2-mingw32-debug
```

#### Linux
```bash
# Release build
cmake --preset linux-gcc
cmake --build --preset linux-gcc

# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

#### macOS
```bash
# Release build
cmake --preset macos-clang
cmake --build --preset macos-clang

# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Manual Build Process

1. **Create build directory**
   ```bash
   mkdir build
   cd build
   ```

2. **Configure with CMake**
   ```bash
   # Windows (MSYS2 MinGW32)
   cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
   
   # Linux
   cmake .. -DCMAKE_BUILD_TYPE=Release
   
   # macOS
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

3. **Build the project**
   ```bash
   # Windows (MSYS2 MinGW32)
   mingw32-make -j4
   
   # Linux/macOS
   make -j4
   
   # Or using CMake
   cmake --build . --parallel
   ```

### Using Ninja (Optional)

1. **Install Ninja**
   ```bash
   # Windows (MSYS2)
   pacman -S mingw-w64-i686-ninja
   
   # Linux
   sudo apt install ninja-build
   
   # macOS
   brew install ninja
   ```

2. **Configure with Ninja**
   ```bash
   cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
   ```

3. **Build with Ninja**
   ```bash
   ninja
   ```

## Running Tests

### Using CTest
```bash
# Run all tests
ctest --preset <preset-name>

# Run with verbose output
ctest --preset <preset-name> --verbose

# Run specific test
ctest --preset <preset-name> -R system_tests
```

### Direct Execution
```bash
# Windows
./build-msys2-mingw32/tests/test_system.exe

# Linux/macOS
./build-linux-gcc/tests/test_system
```

## Running the Application

### Basic Usage
```bash
# Windows
./build-msys2-mingw32/bin/aiaudio_generator.exe --demo

# Linux/macOS
./build-linux-gcc/bin/aiaudio_generator --demo
```

### Command Line Options
```bash
# Show help
./aiaudio_generator --help

# Load preset and generate audio
./aiaudio_generator --preset my_preset.json --output output.txt --duration 10.0

# Run tests
./aiaudio_generator --test

# Run demo
./aiaudio_generator --demo
```

## Troubleshooting

### Common Build Issues

1. **yaml-cpp not found**
   ```
   CMake Error: Could not find yaml-cpp
   ```
   **Solution**: Install yaml-cpp development package for your platform

2. **CMake version too old**
   ```
   CMake 3.20 or higher is required
   ```
   **Solution**: Update CMake to version 3.20 or higher

3. **Compiler not found**
   ```
   No CMAKE_CXX_COMPILER could be found
   ```
   **Solution**: Install C++ compiler and ensure it's in PATH

4. **Windows: make: cmd.exe: No such file or directory**
   ```
   make: cmd.exe: No such file or directory
   ```
   **Solution**: Use `mingw32-make` instead of `make` on Windows

### Platform-Specific Issues

#### Windows (MSYS2 MinGW32)
- Ensure you're using the MinGW32 shell, not MSYS2
- Use `mingw32-make` instead of `make`
- Check that all dependencies are installed for MinGW32

#### Linux
- Ensure development packages are installed (not just runtime)
- Check that CMake can find yaml-cpp
- Verify compiler version supports C++17

#### macOS
- Ensure Xcode Command Line Tools are installed
- Check that Homebrew packages are in PATH
- Verify CMake version is 3.20+

### Debug Build Issues

1. **Debug symbols not generated**
   - Ensure `CMAKE_BUILD_TYPE=Debug` is set
   - Check compiler flags include debug information

2. **Performance issues in debug build**
   - Debug builds are slower by design
   - Use release build for performance testing

### Testing Issues

1. **Tests fail to run**
   - Check that test executable was built
   - Verify working directory is correct
   - Check for missing dependencies

2. **Test failures**
   - Run with verbose output to see details
   - Check that all required libraries are linked
   - Verify test data is accessible

## Advanced Configuration

### Custom CMake Options

```bash
# Enable verbose output
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON

# Set custom install prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local

# Enable specific features
cmake .. -DENABLE_TESTS=ON -DENABLE_DOCS=ON

# Set custom compiler
cmake .. -DCMAKE_CXX_COMPILER=g++-9
```

### Cross-Compilation

```bash
# Set toolchain file
cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/toolchain.cmake

# Set target system
cmake .. -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=x86_64
```

## Performance Optimization

### Release Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### Compiler Optimization
```bash
# GCC
cmake .. -DCMAKE_CXX_FLAGS="-O3 -march=native"

# Clang
cmake .. -DCMAKE_CXX_FLAGS="-O3 -march=native"

# MSVC
cmake .. -DCMAKE_CXX_FLAGS="/O2 /arch:AVX2"
```

### Link-Time Optimization
```bash
# GCC
cmake .. -DCMAKE_CXX_FLAGS="-flto" -DCMAKE_EXE_LINKER_FLAGS="-flto"

# Clang
cmake .. -DCMAKE_CXX_FLAGS="-flto" -DCMAKE_EXE_LINKER_FLAGS="-flto"
```

## Continuous Integration

### GitHub Actions Example
```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [windows-latest, ubuntu-latest, macos-latest]
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        # Windows
        if: runner.os == 'Windows'
        pacman -S mingw-w64-i686-yaml-cpp
        
        # Linux
        if: runner.os == 'Linux'
        sudo apt-get install libyaml-cpp-dev
        
        # macOS
        if: runner.os == 'macOS'
        brew install yaml-cpp
    
    - name: Configure
      run: cmake --preset ${{ matrix.preset }}
    
    - name: Build
      run: cmake --build --preset ${{ matrix.preset }}
    
    - name: Test
      run: ctest --preset ${{ matrix.preset }}
```

This completes the comprehensive build instructions for the AI Synthesizer project.