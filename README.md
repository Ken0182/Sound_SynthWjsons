# AI Audio Generator - Modern C++ Implementation

A comprehensive AI-controlled audio synthesizer built in modern C++ with a focus on cross-platform compatibility and stable builds.

## 🎯 Project Goals

This project recreates the AI-controlled audio synthesizer with the following objectives:

1. **Stable Build Pipeline** - Works cleanly on Windows (MSYS2 MinGW32), macOS, and Linux
2. **Modern C++** - Uses C++20 features and best practices
3. **Minimal Dependencies** - Self-contained with bundled JSON parser
4. **Cross-Platform** - CMake presets for different platforms and compilers
5. **Production Ready** - Comprehensive error handling and validation

## 🚀 Quick Start

### Prerequisites

- **C++20 Compiler** (GCC 10+, Clang 12+, or MSVC 2019+)
- **CMake 3.20+**
- **Make** or **Ninja** (optional)

### Build and Run

```bash
# Clone and navigate to the project
git clone <repository-url>
cd ai-audio-generator

# Build and run (Linux/macOS)
./build_and_run.sh

# Or manually:
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./src/aiaudio_generator
```

### Windows (MSYS2 MinGW)

```bash
# In MSYS2 terminal
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-make

# Build
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./src/aiaudio_generator.exe
```

## 🏗️ Build System

### CMake Presets

The project includes CMake presets for different platforms:

```bash
# List available presets
cmake --list-presets

# Configure with specific preset
cmake --preset=msys2-mingw
cmake --preset=ninja
cmake --preset=debug

# Build with preset
cmake --build --preset=default
```

### Available Presets

- `default` - Standard Unix Makefiles build
- `debug` - Debug build with symbols
- `ninja` - Ninja generator for faster builds
- `msys2-mingw` - MSYS2 MinGW on Windows

### Build Types

- **Release** (default) - Optimized for performance
- **Debug** - Includes debug symbols and assertions

## 📁 Project Structure

```
├── CMakeLists.txt          # Main CMake configuration
├── CMakePresets.json       # CMake presets for different platforms
├── build_and_run.sh        # Quick build and test script
├── include/                # Header files
│   ├── core_types.h        # Core type definitions
│   ├── dsp_ir.h           # DSP intermediate representation
│   ├── simple_json.h      # Bundled JSON parser
│   └── ...
├── src/                   # Source files
│   ├── core_types.cpp     # Core type implementations
│   ├── dsp_ir.cpp        # DSP IR implementation
│   ├── main.cpp          # Main application
│   └── ...
├── tests/                 # Test suite (when GTest is available)
└── config/               # Configuration files
    ├── metrics.yaml      # Metrics configuration
    └── roles.yaml        # Role policies
```

## 🔧 Core Features

### ✅ Implemented

- **DSP Graph System** - Modular audio processing with typed parameters
- **JSON Parser** - Self-contained JSON parsing for preset loading
- **Audio Processing** - Oscillators, filters, envelopes, and LFOs
- **Cross-Platform Build** - Works on Windows, macOS, and Linux
- **CMake Presets** - Easy configuration for different platforms

### 🚧 Optional Features (when dependencies available)

- **OpenCV Integration** - Computer vision utilities (optional)
- **YAML Configuration** - YAML-based configuration (optional)
- **Unit Testing** - Google Test integration (optional)

## 🎵 Usage Examples

### Basic DSP Graph Creation

```cpp
#include "core_types.h"
#include "dsp_ir.h"

using namespace aiaudio;

// Create a simple oscillator
auto osc = std::make_unique<OscillatorStage>();
osc->setParameter("frequency", 440.0);
osc->setParameter("amplitude", 0.5);
osc->setParameter("waveType", std::string("sine"));

// Create a filter
auto filter = std::make_unique<FilterStage>();
filter->setParameter("cutoff", 1000.0);
filter->setParameter("resonance", 0.3);

// Create DSP graph
DSPGraph graph;
graph.addStage("osc1", std::move(osc));
graph.addStage("filter1", std::move(filter));

// Process audio
AudioBuffer input(1024, 0.0);
AudioBuffer output;
graph.process(input, output);
```

### JSON Preset Loading

```cpp
#include "dsp_ir.h"

// JSON preset data
std::string jsonPreset = R"({
    "stages": {
        "oscillator": {
            "type": "oscillator",
            "parameters": {
                "frequency": 880.0,
                "amplitude": 0.7,
                "waveType": "square"
            }
        }
    },
    "connections": []
})";

// Parse and create graph
IRParser parser;
auto graph = parser.parsePreset(jsonPreset);
```

## 🛠️ Development

### Adding New DSP Stages

1. Create a new class inheriting from `DSPStage`
2. Implement the required virtual methods
3. Add to the `createStageFromJSON` function in `IRParser`
4. Update the JSON schema documentation

### Extending JSON Support

The bundled `simple_json.h` provides a lightweight JSON parser. To extend it:

1. Add new parsing methods to the `Reader` class
2. Add new value types to the `Value` class
3. Update the `IRParser` to use new features

### Platform-Specific Code

Use CMake's platform detection:

```cpp
#ifdef _WIN32
    // Windows-specific code
#elif __APPLE__
    // macOS-specific code
#elif __linux__
    // Linux-specific code
#endif
```

## 🐛 Troubleshooting

### Common Build Issues

1. **CMake not found**
   ```bash
   # Ubuntu/Debian
   sudo apt install cmake
   
   # macOS
   brew install cmake
   
   # Windows (MSYS2)
   pacman -S mingw-w64-x86_64-cmake
   ```

2. **C++20 not supported**
   - Update your compiler to GCC 10+, Clang 12+, or MSVC 2019+
   - Check with: `g++ --version` or `clang++ --version`

3. **Build fails on Windows**
   - Use MSYS2 MinGW: `cmake .. -G "MinGW Makefiles"`
   - Ensure you're using the correct generator for your setup

4. **JSON parsing errors**
   - The bundled JSON parser is basic but functional
   - For complex JSON, consider adding a full JSON library

### Debug Build

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
gdb ./src/aiaudio_generator  # or lldb on macOS
```

## 📋 Dependencies

### Required
- C++20 compiler
- CMake 3.20+
- Make or Ninja

### Optional
- OpenCV (for computer vision features)
- yaml-cpp (for YAML configuration)
- Google Test (for unit testing)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on multiple platforms
5. Submit a pull request

## 📄 License

This project is part of the AI Audio Generator system. See the main project documentation for licensing information.

## 🆘 Support

For issues and questions:

1. Check the troubleshooting section
2. Run `./build_and_run.sh` to test the system
3. Check the console output for error messages
4. Verify all dependencies are properly installed

## 🎯 Future Enhancements

- **VST Plugin Support** - Real-time plugin interface
- **Web Interface** - Browser-based generation
- **Advanced ML Models** - ONNX integration
- **Real-time Audio** - Low-latency processing
- **Cloud Deployment** - Scalable cloud service

---

**Status**: ✅ Core functionality working, ready for development and testing