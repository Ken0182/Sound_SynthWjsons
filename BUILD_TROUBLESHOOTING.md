# Build Troubleshooting Guide

## Common Build Issues and Solutions

### 1. CMake Cache Mismatch Error

**Error Message:**
```
CMake Error: The current CMakeCache.txt directory is different than the directory where CMakeCache.txt was created.
```

**Solution:**
This happens when you try to build in a different directory than where CMake was originally configured. 

**Option A - Use the Makefile (Recommended):**
```bash
make rebuild-cpp
```

**Option B - Manual cleanup:**
```bash
rm -rf build/*
# or on Windows
rmdir /s build
mkdir build

make build-cpp
# or manually:
cd build && cmake .. && make
```

### 2. Missing Dependencies (jsoncpp, yaml-cpp)

**Error Message:**
```
Package 'jsoncpp', required by 'virtual:world', not found
```

**Solutions by Platform:**

#### Linux (Debian/Ubuntu):
```bash
sudo apt-get update
sudo apt-get install libjsoncpp-dev libyaml-cpp-dev cmake g++ pkg-config
```

#### macOS:
```bash
brew install jsoncpp yaml-cpp cmake
```

#### Windows (MINGW/MSYS2):
```bash
pacman -S mingw-w64-x86_64-jsoncpp mingw-w64-x86_64-yaml-cpp mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc
```

#### Windows (vcpkg):
```bash
vcpkg install jsoncpp yaml-cpp
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake
```

### 3. Cross-Platform Build Issues

The build system now automatically detects your platform:
- On MINGW/MSYS: Uses portable compiler flags
- On Linux/macOS: Uses optimized flags including `-march=native`

### 4. Quick Build Commands

#### Clean rebuild (recommended when switching machines):
```bash
make rebuild-cpp
```

#### Normal build:
```bash
make build-cpp
```

#### Full clean:
```bash
make clean
```

### 5. Build from Different Directory

If you cloned the project to a different location:

1. **Always clean the build directory first:**
   ```bash
   rm -rf build/*
   ```

2. **Then build normally:**
   ```bash
   make build-cpp
   ```

### 6. Verifying Your Environment

Check if dependencies are installed:

```bash
# Check for jsoncpp
pkg-config --modversion jsoncpp

# Check for yaml-cpp
pkg-config --modversion yaml-cpp

# Check CMake version
cmake --version
```

### 7. Platform-Specific Notes

#### Windows/MINGW:
- Make sure you're using MINGW64 (not MINGW32) for 64-bit builds
- Use forward slashes or escaped backslashes in paths
- Some features may require MSYS2 environment

#### Linux:
- Install build-essential if you haven't: `sudo apt-get install build-essential`
- Make sure pkg-config is installed

#### macOS:
- Install Xcode command line tools: `xcode-select --install`
- Use Homebrew for dependencies

## Still Having Issues?

1. Try a completely clean build:
   ```bash
   make clean
   make build-cpp
   ```

2. Check that all dependencies are installed correctly

3. Make sure you're using a compatible compiler (GCC 10+ or Clang 12+)

4. Verify CMake version is 3.20 or higher
