# Windows Build Instructions for AI Audio Generator

## Prerequisites
- Windows 10/11
- MSYS2 MINGW32 installed
- Git (for cloning the repository)

## Step 1: Install MSYS2
1. Download MSYS2 from https://www.msys2.org/
2. Install and update the base system:
   ```bash
   pacman -Syu
   ```

## Step 2: Install Required Packages
Open MSYS2 MINGW32 shell and install dependencies:

```bash
# Update package database
pacman -Syu

# Install build tools
pacman -S mingw-w64-i686-cmake
pacman -S mingw-w64-i686-make
pacman -S mingw-w64-i686-gcc

# Install required libraries
pacman -S mingw-w64-i686-jsoncpp
pacman -S mingw-w64-i686-yaml-cpp
pacman -S mingw-w64-i686-pkg-config

# Verify installations
pkg-config --cflags --libs jsoncpp
pkg-config --cflags --libs yaml-cpp
```

## Step 3: Clone and Build
```bash
# Clone the repository
git clone <repository-url>
cd ai-audio-generator

# Clean any existing build
rm -rf build/

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# Build the project
make -j4
```

## Step 4: Verify Build
```bash
# Check if executable was created
ls src/aiaudio_generator.exe

# Run a basic test
./src/aiaudio_generator.exe --help
```

## Troubleshooting

### Issue: jsoncpp string_view errors
**Error**: `undefined reference to Json::Value::isMember(std::basic_string_view...)`

**Solution**: This should be fixed in the current code. If you still see this error:
1. Ensure you're using the latest code with the jsoncpp fix
2. Clean and rebuild: `make clean && make`

### Issue: yaml-cpp not found
**Error**: `Package 'yaml-cpp', required by 'virtual:world', not found`

**Solution**:
```bash
pacman -S mingw-w64-i686-yaml-cpp
```

### Issue: CMake can't find jsoncpp
**Error**: `Could not find jsoncpp`

**Solution**:
```bash
# Install jsoncpp development package
pacman -S mingw-w64-i686-jsoncpp

# Verify installation
pkg-config --cflags --libs jsoncpp
```

### Issue: Build fails with permission errors
**Solution**:
1. Run MSYS2 as Administrator
2. Or build in a directory outside Program Files

## Expected Build Output
A successful build should create:
- `src/aiaudio_generator.exe` - Main executable
- `src/libaiaudio_core.a` - Core library
- Various object files in build directories

## Testing the Fix
To verify the jsoncpp fix is working:
1. Build should complete without linker errors
2. No `string_view` related undefined references
3. Executable should run without crashes

## Next Steps After Successful Build
1. Test basic functionality
2. Run any included tests
3. Try generating audio presets
4. Verify JSON parsing works correctly