# Clean Restart Plan for AI Audio Generator

## Overview
This document outlines a systematic approach to get the AI Audio Generator working properly on Windows with MinGW, addressing the jsoncpp linking issues and ensuring a clean, maintainable codebase.

## Phase 1: Environment Setup
### 1.1 Verify MSYS2 Installation
- [ ] Confirm MSYS2 MINGW32 is properly installed
- [ ] Verify all required packages are available:
  - `mingw-w64-i686-jsoncpp`
  - `mingw-w64-i686-yaml-cpp`
  - `mingw-w64-i686-cmake`
  - `mingw-w64-i686-make`

### 1.2 Clean Workspace
- [ ] Remove all build artifacts (`build/` directory)
- [ ] Clean any cached CMake files
- [ ] Ensure we're on the correct branch

## Phase 2: Dependency Verification
### 2.1 Check jsoncpp Installation
```bash
# In MSYS2 MINGW32 shell
pacman -S mingw-w64-i686-jsoncpp
pkg-config --cflags --libs jsoncpp
```

### 2.2 Check yaml-cpp Installation
```bash
# In MSYS2 MINGW32 shell
pacman -S mingw-w64-i686-yaml-cpp
pkg-config --cflags --libs yaml-cpp
```

### 2.3 Verify Headers Location
- [ ] Confirm jsoncpp headers are in `/mingw32/include/jsoncpp/`
- [ ] Confirm yaml-cpp headers are in `/mingw32/include/`

## Phase 3: Code Fixes
### 3.1 Apply jsoncpp Fix (Already Done)
- ✅ Replace `Json::String("key")` with `std::string("key")`
- ✅ Ensure all JSON access uses explicit std::string constructors

### 3.2 Verify Fix Completeness
- [ ] Check all files for jsoncpp usage
- [ ] Ensure no string literals are used directly with JSON methods
- [ ] Test compilation of individual files

## Phase 4: Build System Fixes
### 4.1 Update CMakeLists.txt
- [ ] Add proper MinGW-specific configurations
- [ ] Ensure correct library linking order
- [ ] Add fallback paths for dependencies

### 4.2 Create Windows-Specific Build Script
- [ ] Create `build_windows.bat` or `build_windows.sh`
- [ ] Include dependency checks
- [ ] Add proper error handling

## Phase 5: Testing and Validation
### 5.1 Incremental Build Testing
- [ ] Test compilation of core library first
- [ ] Test compilation of main executable
- [ ] Test linking phase

### 5.2 Runtime Testing
- [ ] Test basic functionality
- [ ] Test JSON parsing with sample data
- [ ] Verify no runtime errors

## Phase 6: Documentation and Maintenance
### 6.1 Update Build Instructions
- [ ] Document Windows build requirements
- [ ] Create troubleshooting guide
- [ ] Add dependency installation steps

### 6.2 Code Quality
- [ ] Add comments explaining the jsoncpp fix
- [ ] Ensure consistent coding style
- [ ] Add error handling where needed

## Emergency Fallback Plan
If the above doesn't work:
1. Consider using a different JSON library (nlohmann/json)
2. Use static linking for problematic libraries
3. Create a Windows-specific build configuration

## Success Criteria
- [ ] Clean build with no linker errors
- [ ] All tests pass
- [ ] Basic functionality works
- [ ] Documentation is updated