# AI Synthesizer - Modern C++ Audio Engine

A modern, cross-platform C++ audio synthesizer engine designed for AI-driven sound generation. This project addresses the common issues with `jsoncpp` string_view compatibility and provides a robust, portable solution for audio synthesis.

## Features

- **Modern C++17** implementation with clean, maintainable code
- **Portable JSON parsing** using a custom implementation that avoids `jsoncpp` string_view issues
- **Cross-platform support** for Windows (MSYS2 MinGW32), Linux, and macOS
- **Modular DSP architecture** with pluggable audio stages
- **Real-time audio processing** with thread-safe operations
- **Comprehensive testing** using CTest framework
- **Multiple build systems** support (CMake, Ninja, Make)
- **No external dependencies** beyond system libraries

## Quick Start

### Windows (MSYS2 MinGW32)

```bash
# Install dependencies
pacman -S mingw-w64-i686-yaml-cpp

# Configure and build
cmake --preset windows-msys2-mingw32
cmake --build --preset windows-msys2-mingw32

# Run tests
ctest --preset windows-msys2-mingw32

# Run demo
./build-msys2-mingw32/bin/aiaudio_generator --demo
```

### Linux

```bash
# Install dependencies
sudo apt-get install libyaml-cpp-dev

# Configure and build
cmake --preset linux-gcc
cmake --build --preset linux-gcc

# Run tests
ctest --preset linux-gcc

# Run demo
./build-linux-gcc/bin/aiaudio_generator --demo
```

### macOS

```bash
# Install dependencies
brew install yaml-cpp

# Configure and build
cmake --preset macos-clang
cmake --build --preset macos-clang

# Run tests
ctest --preset macos-clang

# Run demo
./build-macos-clang/bin/aiaudio_generator --demo
```

## Architecture

### Core Components

1. **DSP Stages** - Modular audio processing units
   - `OscillatorStage` - Waveform generation
   - `FilterStage` - Frequency filtering
   - `EnvelopeStage` - Amplitude modulation
   - `LFOStage` - Low-frequency oscillation

2. **DSP Graph** - Audio signal routing and processing
   - Topological sorting for proper stage ordering
   - Cycle detection and validation
   - Connection management

3. **IR Parser** - Preset loading and saving
   - Portable JSON implementation
   - No `jsoncpp` string_view dependencies
   - Robust error handling

4. **Main Application** - High-level API
   - Thread-safe operations
   - Real-time parameter control
   - Audio processing pipeline

### Key Design Decisions

- **Portable JSON**: Custom implementation avoids `jsoncpp` string_view issues
- **Modern C++**: Uses C++17 features for better performance and safety
- **Thread Safety**: All operations are thread-safe for real-time use
- **Modular Design**: Easy to extend with new DSP stages
- **Cross-Platform**: Works on Windows, Linux, and macOS

## Usage

### Basic Audio Generation

```cpp
#include "main_app.h"

using namespace aiaudio;

// Create application
MainApp app;

// Add oscillator stage
auto osc = std::make_unique<OscillatorStage>();
osc->setParameter("frequency", 440.0);
osc->setParameter("amplitude", 0.5);
osc->setParameter("waveType", std::string("sine"));
app.addStage("osc1", std::move(osc));

// Process audio
AudioBuffer input(1024, 0.0);
AudioBuffer output;
app.processAudio(input, output);
```

### Loading Presets

```cpp
// Load preset from JSON file
if (app.loadPreset("my_preset.json")) {
    std::cout << "Preset loaded successfully\n";
} else {
    std::cout << "Error: " << app.getLastError() << "\n";
}
```

### Real-time Parameter Control

```cpp
// Set parameter in real-time
app.setParameter("osc1", "frequency", 880.0);

// Get current parameter value
auto freq = app.getParameter("osc1", "frequency");
double frequency = std::get<double>(freq);
```

## Preset Format

Presets are stored in JSON format with the following structure:

```json
{
  "stages": {
    "osc1": {
      "type": "oscillator",
      "parameters": {
        "frequency": 440.0,
        "amplitude": 0.5,
        "waveType": "sine"
      }
    },
    "filter1": {
      "type": "filter",
      "parameters": {
        "cutoff": 1000.0,
        "resonance": 0.1,
        "filterType": "lowpass"
      }
    }
  },
  "connections": [
    {
      "source": "osc1",
      "destination": "filter1",
      "parameter": "input",
      "amount": 1.0,
      "enabled": true
    }
  ]
}
```

## Building

### CMake Presets

The project includes several CMake presets for different platforms:

- `windows-msys2-mingw32` - Windows with MSYS2 MinGW32
- `windows-msys2-mingw32-debug` - Debug build for Windows
- `linux-gcc` - Linux with GCC
- `macos-clang` - macOS with Clang
- `ninja-multi` - Multi-config Ninja build

### Custom Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### Dependencies

- **yaml-cpp** - YAML configuration parsing
- **C++17 compiler** - GCC 7+, Clang 5+, or MSVC 2017+
- **CMake 3.20+** - Build system

## Testing

Run the test suite:

```bash
# Using CTest
ctest --preset <preset-name>

# Or directly
./build-<platform>/tests/test_system
```

## Troubleshooting

### Common Issues

1. **yaml-cpp not found**
   - Windows: `pacman -S mingw-w64-i686-yaml-cpp`
   - Linux: `sudo apt-get install libyaml-cpp-dev`
   - macOS: `brew install yaml-cpp`

2. **Build errors on Windows**
   - Use `mingw32-make` instead of `make`
   - Ensure MSYS2 MinGW32 environment is active

3. **JSON parsing errors**
   - Check preset file format
   - Ensure all required fields are present

### Debug Build

For debugging, use the debug preset:

```bash
cmake --preset windows-msys2-mingw32-debug
cmake --build --preset windows-msys2-mingw32-debug
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Built with modern C++ best practices
- Addresses common `jsoncpp` compatibility issues
- Designed for cross-platform audio development
- Inspired by modular synthesizer architectures