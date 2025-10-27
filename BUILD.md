# AI Audio Generator - Build Instructions

Comprehensive build guide for Windows (MSYS2/MinGW, MSVC), Linux, and macOS.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Windows Build (MSYS2/MinGW)](#windows-build-msys2mingw)
- [Windows Build (MSVC/Visual Studio)](#windows-build-msvcvisual-studio)
- [Linux Build](#linux-build)
- [macOS Build](#macos-build)
- [CMake Presets](#cmake-presets)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required

- **C++20 compatible compiler**
  - GCC 10+ 
  - Clang 12+
  - MSVC 2019 16.11+ (Visual Studio 2019)
  - MinGW-w64 GCC 10+

- **CMake 3.20 or later**

- **nlohmann/json 3.2.0+** (header-only JSON library)

### Optional

- **OpenCV 4.x** - For advanced features (disabled if not found)
- **Google Test** - For running unit tests (disabled if not found)
- **Ninja** - Fast build system (optional, can use Make/MSVC)

---

## Windows Build (MSYS2/MinGW)

### Step 1: Install MSYS2

Download and install MSYS2 from https://www.msys2.org/

### Step 2: Install Build Tools

Open **MSYS2 MinGW 64-bit** terminal and run:

```bash
# Update package database
pacman -Syu

# Install build tools
pacman -S mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-ninja \
          mingw-w64-x86_64-nlohmann-json \
          make

# Optional: Install OpenCV and GTest
pacman -S mingw-w64-x86_64-opencv \
          mingw-w64-x86_64-gtest
```

### Step 3: Build with CMake Presets

```bash
# Navigate to project directory
cd /path/to/AIAudioGenerator

# Configure and build using MinGW preset
cmake --preset mingw-release
cmake --build build/mingw-release

# Or for debug build
cmake --preset mingw-debug
cmake --build build/mingw-debug
```

### Step 4: Manual Build (Alternative)

If you prefer manual configuration:

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Step 5: Run the Application

```bash
./build/mingw-release/src/aiaudio_generator.exe
```

---

## Windows Build (MSVC/Visual Studio)

### Step 1: Install Visual Studio

Install Visual Studio 2019 or later with:
- Desktop development with C++
- CMake tools for Windows

### Step 2: Install vcpkg (Recommended)

```powershell
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install opencv4:x64-windows        # Optional
.\vcpkg install gtest:x64-windows          # Optional

# Integrate with Visual Studio
.\vcpkg integrate install
```

### Step 3: Build with CMake

```powershell
# Using CMake presets
cmake --preset msvc-release -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build build/msvc-release --config Release

# Or open in Visual Studio
# File > Open > CMake > select CMakeLists.txt
```

### Step 4: Alternative - Command Line Build

```powershell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake ^
      ..
cmake --build . --config Release
```

---

## Linux Build

### Step 1: Install Dependencies

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    nlohmann-json3-dev \
    libopencv-dev \
    libgtest-dev
```

#### Fedora/RHEL

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    json-devel \
    opencv-devel \
    gtest-devel
```

#### Arch Linux

```bash
sudo pacman -S base-devel cmake ninja nlohmann-json opencv gtest
```

### Step 2: Build with CMake Presets

```bash
# Using Ninja (recommended)
cmake --preset ninja-release
cmake --build build/ninja-release

# Or using Make
cmake --preset make-release
cmake --build build/make-release
```

### Step 3: Manual Build (Alternative)

```bash
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Or with Make
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Step 4: Run the Application

```bash
./build/ninja-release/src/aiaudio_generator
```

### Step 5: Run Tests (Optional)

```bash
cd build/ninja-release
ctest --output-on-failure
```

---

## macOS Build

### Step 1: Install Xcode Command Line Tools

```bash
xcode-select --install
```

### Step 2: Install Homebrew

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Step 3: Install Dependencies

```bash
brew install cmake ninja nlohmann-json
brew install opencv        # Optional
brew install googletest    # Optional
```

### Step 4: Build

```bash
# Using Ninja
cmake --preset ninja-release
cmake --build build/ninja-release

# Or using Make
cmake --preset make-release
cmake --build build/make-release
```

### Step 5: Run the Application

```bash
./build/ninja-release/src/aiaudio_generator
```

---

## CMake Presets

This project provides CMake presets for common build configurations:

### Available Configure Presets

- `ninja-release` - Ninja generator, Release build
- `ninja-debug` - Ninja generator, Debug build
- `make-release` - Unix Makefiles, Release build
- `make-debug` - Unix Makefiles, Debug build
- `mingw-release` - MinGW Makefiles (Windows), Release
- `mingw-debug` - MinGW Makefiles (Windows), Debug
- `msvc-release` - Visual Studio generator, Release
- `msvc-debug` - Visual Studio generator, Debug

### Usage

```bash
# List available presets
cmake --list-presets

# Configure with a preset
cmake --preset ninja-release

# Build
cmake --build build/ninja-release

# Run tests
ctest --preset ninja-release
```

---

## Build Configuration Options

### Optional Features

You can configure optional features using CMake variables:

```bash
# Disable OpenCV support
cmake --preset ninja-release -DHAVE_OPENCV=0

# Enable verbose output
cmake --preset ninja-release --verbose
```

### Custom Installation Path

```bash
cmake --preset ninja-release -DCMAKE_INSTALL_PREFIX=/custom/path
cmake --build build/ninja-release --target install
```

---

## Troubleshooting

### Issue: `nlohmann/json` not found

**Solution:**

Download the single-header file:
```bash
mkdir -p include/nlohmann
curl -o include/nlohmann/json.hpp https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
```

Then configure with:
```bash
cmake -DCMAKE_INCLUDE_PATH=./include ..
```

### Issue: OpenCV not found

**Solution:**

OpenCV is optional. The build will continue without it, disabling vision-based features. To force OpenCV:

```bash
cmake --preset ninja-release -DOpenCV_DIR=/path/to/opencv/build
```

### Issue: Architecture-specific flags error on Windows

**Solution:**

This has been fixed in the current build system. If you still encounter issues, ensure you're using the latest CMakeLists.txt which removes `-march=native` flags.

### Issue: Undefined reference errors with jsoncpp

**Solution:**

This project uses **nlohmann/json**, not jsoncpp. Ensure you have nlohmann-json installed and that no old jsoncpp headers are being picked up.

### Issue: MinGW linker errors

**Solution:**

Make sure you're using the correct MSYS2 environment:
- Use **MSYS2 MinGW 64-bit** for 64-bit builds
- Use **MSYS2 MinGW 32-bit** for 32-bit builds

### Issue: Permission denied on Windows

**Solution:**

Run MSYS2 or Visual Studio Command Prompt as Administrator, or check antivirus settings.

---

## Performance Optimization

### Release Builds

For optimal performance, always use Release builds:
```bash
cmake --preset ninja-release
```

### Link-Time Optimization (LTO)

Enable LTO for even better performance:
```bash
cmake --preset ninja-release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

### Custom Optimization Flags

The build system automatically applies appropriate optimization flags:
- GCC/Clang: `-O3 -ffast-math` (Release)
- MSVC: `/O2` (Release)
- No architecture-specific flags to ensure portability

---

## Cross-Compilation

### Windows → Linux (WSL)

```bash
# From WSL
cmake --preset make-release
cmake --build build/make-release
```

### Building for Multiple Platforms

Use Docker for consistent cross-platform builds:

```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential cmake nlohmann-json3-dev
COPY . /workspace
WORKDIR /workspace
RUN cmake --preset make-release && cmake --build build/make-release
```

---

## IDE Integration

### Visual Studio Code

The project includes `.vscode` settings (if present). Install:
- C/C++ Extension
- CMake Tools Extension

Configure with:
1. Open Command Palette (Ctrl+Shift+P)
2. "CMake: Select Configure Preset"
3. Choose your platform preset

### CLion

CLion automatically detects CMakePresets.json. Just open the project folder.

### Visual Studio

Open the folder containing CMakeLists.txt:
1. File → Open → CMake
2. VS will automatically configure using presets

---

## Next Steps

After building successfully:

1. **Run Examples**: `./build/ninja-release/src/aiaudio_generator`
2. **Run Tests**: `ctest --test-dir build/ninja-release`
3. **Read Documentation**: See [README.md](README.md) for usage
4. **Explore Presets**: Check `guitar.json`, `group.json` for examples

---

## Getting Help

If you encounter issues not covered here:

1. Check the [main README](README.md)
2. Review [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)
3. Ensure all dependencies are correctly installed
4. Try a clean build: `rm -rf build && cmake --preset ninja-release`

---

## Build Verification

To verify your build is working correctly:

```bash
# Build in release mode
cmake --preset ninja-release
cmake --build build/ninja-release

# Run the application with a test
./build/ninja-release/src/aiaudio_generator

# Expected output: System initialization and test sound generation
```

If you see successful initialization messages, your build is ready!
