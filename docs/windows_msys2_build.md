## Windows MSYS2 Build Guide (MinGW)

This guide explains how to build and run the AI Audio Generator on Windows using MSYS2 with the MinGW toolchain.

### 1) Install MSYS2 and open the MinGW shell
- Download and install MSYS2 from `https://www.msys2.org`.
- Launch the 64-bit MinGW shell: "MSYS2 MinGW x64" (not the MSYS shell).

### 2) Update and install required packages
```bash
pacman -Syu           # update package database and core
# Restart the shell if prompted, then run again:
pacman -Syu

# Build tools
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-pkgconf

# Libraries
pacman -S --needed mingw-w64-x86_64-yaml-cpp mingw-w64-x86_64-jsoncpp

# Optional: pybind11 for Python bindings
pacman -S --needed mingw-w64-x86_64-pybind11

# Optional: Node.js for web (vite)
pacman -S --needed mingw-w64-x86_64-nodejs

# Python environment
pacman -S --needed mingw-w64-x86_64-python mingw-w64-x86_64-python-pip
```

### 3) Clone or open the project
Ensure you are in the MinGW shell and the repo is on a Windows path (not under /c/Users if you prefer, but MSYS2 handles this fine).

```bash
cd /c/path/to/your/workspace
# git clone ...  (if needed)
```

### 4) Configure with CMake Preset (recommended)
We provide a preset for MinGW Makefiles.

```bash
cmake --preset msys2-mingw
cmake --build --preset msys2-mingw -j
```

This will generate build files in `build/mingw` and build the targets:
- `aiaudio_core` (library)
- `aiaudio_generator` (executable)
- `aiaudio_python` (optional pybind11 module if pybind11 is installed)

### 5) Run the C++ demo
```bash
# From the repo root
./build/mingw/src/aiaudio_generator.exe
```

### 6) Python dependencies (optional)
```bash
pip install -r requirements.txt
```

### 7) Web UI (optional)
If you installed Node.js:
```bash
cd web
npm install
npm run build
```

### 8) Troubleshooting
- If CMake cannot find yaml-cpp or jsoncpp, ensure their MinGW packages are installed and you are in the MinGW shell.
- If pybind11 is missing, the Python extension `aiaudio_python` will be skipped automatically.
- If antivirus interferes with build outputs, add the build directory to exceptions.

### 9) Alternative: Makefile wrapper
The top-level `Makefile` delegates to CMake and supports presets:
```bash
# Build using the msys2 preset
make BUILD_PRESET=msys2-mingw build-cpp
```

### 10) Clean builds
```bash
# Remove CMake build artifacts
rm -rf build/mingw
cmake --preset msys2-mingw
cmake --build --preset msys2-mingw -j
```

You're ready to go on Windows/MSYS2!