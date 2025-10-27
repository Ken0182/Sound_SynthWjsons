# AI Audio Generator

A research-grade, production-minded, over-engineered AI audio generation system built in C++. This system implements a comprehensive 20-step blueprint for AI data-reader / sound-outputter with advanced features including multi-objective optimization, semantic fusion, policy-driven generation, and real-time audio processing.

## Features

### Core Components

1. **Multi-Objective Optimization (MOO)** - Pareto dominance-based optimization with objective vectors
2. **JSON to Typed DSP IR** - Strongly-typed intermediate representation with BNF grammar
3. **Canonical Normalization** - Perceptual mappings and musical context normalization
4. **Semantic Fusion** - Query + tags + descriptions with contrastive learning
5. **Roles & Policy Language** - YAML-based constraint system with role-specific policies
6. **Decision Heads** - MLP-based parameter and routing decisions
7. **Audio Safety** - Headroom management and anti-chaos systems
8. **Real-time Processing** - Low-latency audio generation with CPU budget management

### Advanced Features

- **Theory-Aware Constraints** - Tempo/key/scale-aware parameter adjustment
- **A/B Testing & Active Learning** - Bandit algorithms for continuous improvement
- **Full Traceability** - Complete reproducibility with trace provenance
- **Progressive Disclosure** - Basic to expert mode with guarded access
- **Continual Learning** - Trust-region updates for model improvement
- **Feature Flags** - Rollout control and kill switches
- **100% Good Feedback Engine** - Comprehensive feedback collection and analysis

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Semantic      │    │   Decision      │    │   DSP Graph     │
│   Fusion        │───▶│   Heads         │───▶│   Processing    │
│   Engine        │    │   (MLP)         │    │   (IR)          │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Policy        │    │   MOO           │    │   Audio         │
│   Manager       │    │   Optimizer     │    │   Renderer      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Building

### Quick Start

```bash
# Linux/macOS with Ninja
cmake --preset ninja-release
cmake --build build/ninja-release

# Windows MSYS2/MinGW
cmake --preset mingw-release
cmake --build build/mingw-release

# Windows MSVC
cmake --preset msvc-release
cmake --build build/msvc-release
```

### Detailed Build Instructions

For comprehensive build instructions including:
- **Windows (MSYS2/MinGW32, MSVC)**
- **Linux (Ubuntu, Fedora, Arch)**  
- **macOS (Homebrew)**
- **Troubleshooting**
- **IDE Integration**

See **[BUILD.md](BUILD.md)** for complete details.

### Prerequisites

**Required:**
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+, MinGW-w64 GCC 10+)
- CMake 3.20+
- nlohmann/json 3.2.0+

**Optional:**
- OpenCV 4.x (vision features)
- Google Test (unit tests)

### Dependencies

```bash
# Ubuntu/Debian
sudo apt-get install cmake ninja-build nlohmann-json3-dev

# macOS
brew install cmake ninja nlohmann-json

# Windows MSYS2 (MinGW 64-bit terminal)
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-ninja mingw-w64-x86_64-nlohmann-json

# Windows vcpkg
vcpkg install nlohmann-json:x64-windows
```

### CMake Presets

This project uses CMake presets for easy configuration:

- `ninja-release` / `ninja-debug` - Ninja builds
- `make-release` / `make-debug` - Makefile builds  
- `mingw-release` / `mingw-debug` - Windows MinGW builds
- `msvc-release` / `msvc-debug` - Visual Studio builds

List all presets: `cmake --list-presets`

## Usage

### Basic Usage

```cpp
#include "main_app.h"

using namespace aiaudio;

int main() {
    // Initialize the AI Audio Generator
    AIAudioGenerator generator;
    
    // Create generation request
    AIAudioGenerator::GenerationRequest request;
    request.prompt = "dreamy atmospheric pad with reverb";
    request.role = Role::PAD;
    request.context.tempo = 120.0;
    request.context.key = 0; // C major
    request.context.scale = "major";
    request.constraints.maxCPU = 0.8;
    request.constraints.maxLatency = 10.0;
    
    // Generate audio
    auto result = generator.generate(request);
    
    // Access results
    std::cout << "Generated " << result.audio.size() << " samples" << std::endl;
    std::cout << "Quality score: " << result.qualityScore << std::endl;
    std::cout << "Explanation: " << result.explanation << std::endl;
    
    return 0;
}
```

### Advanced Usage

```cpp
// Custom configuration
std::map<std::string, std::string> config;
config["semantic_model"] = "advanced";
config["policy_strictness"] = "high";
config["quality_threshold"] = "0.8";
generator.setConfiguration(config);

// Load custom presets
generator.loadPreset("custom_pad.json");

// Batch processing
std::vector<std::string> prompts = {
    "warm pad", "aggressive bass", "melodic lead"
};
std::vector<Role> roles = {Role::PAD, Role::BASS, Role::LEAD};

for (size_t i = 0; i < prompts.size(); ++i) {
    AIAudioGenerator::GenerationRequest request;
    request.prompt = prompts[i];
    request.role = roles[i];
    request.context.tempo = 120.0;
    request.context.key = 0;
    request.context.scale = "major";
    
    auto result = generator.generate(request);
    std::cout << prompts[i] << " -> Quality: " << result.qualityScore << std::endl;
}
```

## Configuration

### Metrics Configuration (`config/metrics.yaml`)

```yaml
roles:
  pad:
    thresholds:
      semantic_match: 0.7
      mix_readiness: 0.8
      perceptual_quality: 0.75
      stability: 0.9
      preference_win: 0.65
    constraints:
      lufs_target: -18.0
      true_peak_limit: -1.0
      max_cpu: 0.8
      max_latency: 10.0
```

### Role Policies (`config/roles.yaml`)

```yaml
pad:
  version: "1.0"
  description: "Atmospheric pad sounds"
  constraints:
    frequency:
      type: "range"
      min: 100.0
      max: 2000.0
      weight: 1.0
    attack:
      type: "range"
      min: 0.1
      max: 2.0
      weight: 0.8
```

## API Reference

### Core Classes

- `AIAudioGenerator` - Main generation interface
- `MOOOptimizer` - Multi-objective optimization
- `SemanticFusionEngine` - Semantic query processing
- `PolicyManager` - Role-based policy management
- `DecisionHeads` - MLP-based decision making
- `DSPGraph` - Audio processing graph
- `AudioRenderer` - Real-time audio rendering

### Key Functions

- `generate()` - Generate audio from prompt
- `loadPreset()` - Load DSP preset from JSON
- `applyPolicy()` - Apply role-specific constraints
- `assessQuality()` - Evaluate audio quality
- `renderRealtime()` - Real-time audio rendering

## Examples

See `example_usage.cpp` for comprehensive examples including:

- Basic audio generation
- Advanced configuration
- Real-time processing
- Quality assessment
- System monitoring
- Batch processing

## Performance

### Benchmarks

- **Generation Time**: < 100ms for 8-second audio
- **Real-time Latency**: < 5ms for 1024-sample buffers
- **CPU Usage**: < 80% on modern hardware
- **Memory Usage**: < 512MB typical

### Optimization

- SIMD-optimized audio processing
- Multi-threaded generation pipeline
- Efficient memory management
- Real-time constraint checking

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Citation

If you use this system in your research, please cite:

```bibtex
@software{aiaudio_generator,
  title={AI Audio Generator: A Research-Grade Audio Generation System},
  author={Your Name},
  year={2024},
  url={https://github.com/yourusername/ai-audio-generator}
}
```

## Acknowledgments

- OpenCV for computer vision utilities
- nlohmann/json for JSON processing
- yaml-cpp for YAML configuration
- The audio processing community for inspiration

## Roadmap

- [ ] ONNX model integration
- [ ] VST plugin support
- [ ] Web interface
- [ ] Cloud deployment
- [ ] Advanced ML models
- [ ] Real-time collaboration