# Python-C++ Bridge API Documentation

## Overview

The Python-C++ bridge enables seamless communication between the Python semantic engine and the C++ audio rendering system. This document describes the API contract, data flow, and implementation details.

## Architecture

```
Python Layer          Bridge Layer          C++ Layer
┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│ AudioPreset │─────▶│ pybind11    │─────▶│ AIAudioGen  │
│ Generator   │      │ Bindings    │      │ erator      │
└─────────────┘      └─────────────┘      └─────────────┘
│                     │                     │
│ cpp_engine.py       │ aiaudio_python.so   │ src/main_app.cpp
│                     │                     │
└─────────────────────┴─────────────────────┘
```

## API Contract

### Data Types

#### Audio Buffer
- **Python**: `numpy.ndarray` (float32)
- **C++**: `std::vector<float>` or `AudioBuffer`
- **Format**: Mono or stereo, 44.1kHz sample rate

#### Preset Parameters
- **Python**: `Dict[str, Any]`
- **C++**: `std::map<std::string, std::variant<double, std::string>>`
- **Serialization**: JSON-compatible types only

#### Generation Request
- **Python**: Function parameters
- **C++**: `GenerationRequest` struct
- **Threading**: Single-threaded Python → Multi-threaded C++

### Core Functions

#### 1. Audio Generation

```python
# Python Interface
def generate_audio(prompt: str, role: str = "UNKNOWN", 
                  tempo: float = 120.0, key: str = "C", 
                  scale: str = "major", max_cpu: float = 0.8,
                  max_latency: float = 10.0) -> np.ndarray
```

```cpp
// C++ Implementation
GenerationResult generate(const GenerationRequest& request);
```

**Parameters:**
- `prompt`: Text description of desired sound
- `role`: Musical role (PAD, BASS, LEAD, etc.)
- `tempo`: Tempo in BPM
- `key`: Musical key (C, D, E, etc.)
- `scale`: Scale type (major, minor, etc.)
- `max_cpu`: Maximum CPU usage (0.0-1.0)
- `max_latency`: Maximum latency in milliseconds

**Returns:**
- `numpy.ndarray`: Audio samples as float32 array

#### 2. Preset-based Generation

```python
# Python Interface
def generate_from_preset(preset_params: Dict[str, Any], 
                        duration: float = 2.0, 
                        sample_rate: float = 44100.0) -> np.ndarray
```

```cpp
// C++ Implementation
AudioBuffer renderGraph(const DSPGraph& graph, size_t numSamples);
```

**Parameters:**
- `preset_params`: Dictionary of preset parameters
- `duration`: Audio duration in seconds
- `sample_rate`: Sample rate in Hz

**Returns:**
- `numpy.ndarray`: Rendered audio samples

#### 3. System Status

```python
# Python Interface
def get_status() -> Dict[str, Any]
```

```cpp
// C++ Implementation
SystemStatus getStatus() const;
```

**Returns:**
- `Dict[str, Any]`: System status information

## Data Flow

### 1. Preset Loading Flow

```
JSON File → Python Parser → Normalized Preset → C++ DSP Graph → Audio Buffer
```

1. **Python**: Load and parse JSON preset file
2. **Python**: Normalize parameters to SI units
3. **Python**: Apply semantic search and policies
4. **Bridge**: Convert Python dict to C++ map
5. **C++**: Build DSP graph from parameters
6. **C++**: Render audio using real-time engine
7. **Bridge**: Convert C++ vector to numpy array
8. **Python**: Return audio for playback

### 2. Real-time Generation Flow

```
Text Prompt → Semantic Search → Policy Application → C++ Rendering → Audio Output
```

1. **Python**: Generate query embedding
2. **Python**: Search for similar presets
3. **Python**: Apply role-based policies
4. **Bridge**: Transfer decision context to C++
5. **C++**: Generate audio using optimized algorithms
6. **Bridge**: Return audio buffer to Python
7. **Python**: Play audio through audio interface

## Threading Model

### Python Side
- **Main Thread**: UI and API calls
- **Audio Thread**: Playback and real-time audio
- **Worker Threads**: Semantic processing and file I/O

### C++ Side
- **Audio Thread**: Real-time DSP processing
- **Worker Threads**: Graph building and optimization
- **Background Threads**: Quality assessment and monitoring

### Bridge Coordination
- **GIL Release**: Python releases GIL during C++ calls
- **Memory Management**: Shared ownership of audio buffers
- **Error Handling**: Exception translation between languages

## Memory Management

### Audio Buffers
- **Ownership**: C++ owns the buffer, Python gets a view
- **Lifetime**: Managed by pybind11 smart pointers
- **Copying**: Minimal copying, prefer move semantics

### Preset Data
- **Serialization**: JSON-compatible types only
- **Validation**: Type checking at bridge boundary
- **Cleanup**: Automatic cleanup via RAII

## Error Handling

### Python Exceptions
```python
try:
    audio = engine.generate_audio("test")
except RuntimeError as e:
    print(f"Generation failed: {e}")
```

### C++ Exceptions
```cpp
try {
    auto result = generator.generate(request);
} catch (const AIAudioException& e) {
    // Handle audio-specific errors
} catch (const std::exception& e) {
    // Handle general errors
}
```

### Bridge Translation
- **C++ → Python**: `std::exception` → `RuntimeError`
- **Python → C++**: `ValueError` → `std::invalid_argument`
- **Audio Errors**: Specialized exception types

## Performance Considerations

### Latency Optimization
- **Buffer Size**: 64-512 samples for low latency
- **Threading**: Minimize context switches
- **Memory**: Avoid unnecessary copies
- **Caching**: Cache compiled DSP graphs

### CPU Usage
- **Profiling**: Monitor CPU usage per component
- **Optimization**: Use SIMD instructions in C++
- **Load Balancing**: Distribute work across threads
- **Quality vs Speed**: Configurable quality settings

### Memory Usage
- **Buffer Pool**: Reuse audio buffers
- **Smart Pointers**: Automatic memory management
- **Garbage Collection**: Minimize Python GC pressure
- **Memory Mapping**: Use memory-mapped files for large datasets

## Testing Strategy

### Unit Tests
- **Python**: Test individual functions
- **C++**: Test core algorithms
- **Bridge**: Test data conversion

### Integration Tests
- **End-to-End**: Full pipeline testing
- **Performance**: Latency and CPU usage
- **Stress**: Long-running stability tests

### Mock Testing
- **C++ Mock**: Test Python without C++ engine
- **Python Mock**: Test C++ without Python dependencies
- **Bridge Mock**: Test data conversion in isolation

## Build Integration

### CMake Configuration
```cmake
# Find pybind11
find_package(pybind11 REQUIRED)

# Create Python module
pybind11_add_module(aiaudio_python src/python_bindings.cpp)

# Link libraries
target_link_libraries(aiaudio_python PRIVATE aiaudio_core)
```

### Python Installation
```bash
# Build and install
pip install -e .

# Or build manually
python setup.py build_ext --inplace
```

### Development Setup
```bash
# Install dependencies
pip install pybind11 numpy

# Build C++ library
mkdir build && cd build
cmake .. && make

# Install Python module
pip install -e .
```

## Troubleshooting

### Common Issues

1. **Import Error**: C++ module not found
   - **Solution**: Build the project first
   - **Check**: `ls build/aiaudio_python.*`

2. **Memory Error**: Audio buffer corruption
   - **Solution**: Check buffer size and alignment
   - **Debug**: Use valgrind or AddressSanitizer

3. **Latency Issues**: High audio latency
   - **Solution**: Reduce buffer size, optimize C++ code
   - **Check**: Monitor real-time performance

4. **Threading Issues**: Deadlocks or race conditions
   - **Solution**: Review threading model
   - **Debug**: Use thread sanitizer

### Debug Tools

- **Python**: `pdb`, `cProfile`, `memory_profiler`
- **C++**: `gdb`, `valgrind`, `perf`
- **Bridge**: `pybind11` debug macros
- **Audio**: `JACK`, `PulseAudio` monitoring

## Future Enhancements

### Planned Features
- **Async Generation**: Non-blocking audio generation
- **Streaming**: Real-time audio streaming
- **GPU Acceleration**: CUDA/OpenCL support
- **Plugin System**: Dynamic loading of audio effects

### API Evolution
- **Versioning**: Semantic versioning for API changes
- **Backward Compatibility**: Maintain old API versions
- **Migration Guide**: Help users upgrade
- **Deprecation Policy**: Clear deprecation timeline

## Conclusion

The Python-C++ bridge provides a robust, high-performance interface between the semantic engine and audio rendering system. The design prioritizes low latency, memory efficiency, and ease of use while maintaining the flexibility to evolve with changing requirements.

Key success factors:
- Clear API contract and data flow
- Efficient memory management
- Robust error handling
- Comprehensive testing
- Good documentation and tooling