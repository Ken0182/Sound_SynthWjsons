# Python-C++ Bridge API Documentation

## Overview

The Python-C++ bridge provides seamless integration between the Python audio interface and the C++ audio rendering engine. It uses pybind11 to expose C++ functionality to Python with minimal overhead.

## Architecture

```
┌───────────────────────────┐
│     Python Layer          │
│  - audio_interface.py     │
│  - web_server.py          │
│  - cpp_engine.py          │
└─────────┬─────────────────┘
          │ Python-C++ Bridge
          │ (pybind11)
┌─────────▼─────────────────┐
│     C++ Layer             │
│  - python_bindings.cpp    │
│  - AIAudioGenerator       │
│  - DSPGraph               │
└───────────────────────────┘
```

## Installation

### Prerequisites

```bash
# Install pybind11
pip install pybind11

# Ubuntu/Debian
sudo apt-get install pybind11-dev

# macOS
brew install pybind11

# MSYS2
pacman -S mingw-w64-x86_64-pybind11
```

### Build Instructions

```bash
# Configure with Python bindings
mkdir build && cd build
cmake ..
make -j$(nproc)

# Install Python module
make install
# Or manually copy the module
cp aiaudio_cpp*.so /path/to/python/site-packages/
```

## Python API

### CPPAudioEngineAdapter

Main adapter class that provides a clean Python interface to the C++ engine.

#### Constructor

```python
from cpp_engine import CPPAudioEngineAdapter

engine = CPPAudioEngineAdapter(fallback_to_python=True)
```

**Parameters:**
- `fallback_to_python` (bool): If True, fall back to Python implementation when C++ engine is unavailable

#### render_audio()

Render audio from a preset using the C++ engine.

```python
audio = engine.render_audio(
    preset={
        "name": "Warm Pad",
        "role": "pad",
        "description": "Warm atmospheric pad",
        "category": "pad",
        "parameters": {
            "frequency": 440.0,
            "amplitude": 0.5
        }
    },
    context={
        "tempo": 120.0,
        "key": 0,
        "scale": "major",
        "time_signature": [4, 4]
    },
    duration=2.0
)
```

**Parameters:**
- `preset` (Dict[str, Any]): Preset dictionary
  - `name` (str): Preset name
  - `role` (str): Role (pad, bass, lead, drum, fx)
  - `description` (str): Description used as prompt
  - `category` (str): Category/role
  - `parameters` (dict): Audio parameters
  - `constraints` (dict, optional): Rendering constraints
- `context` (Dict[str, Any], optional): Musical context
  - `tempo` (float): BPM (default: 120.0)
  - `key` (int): Key (0=C, 1=C#, ..., 11=B) (default: 0)
  - `scale` (str): Scale ("major" or "minor") (default: "major")
  - `time_signature` (List[int]): Time signature [num, denom] (default: [4, 4])
- `duration` (float): Duration in seconds (default: 2.0)

**Returns:**
- `np.ndarray`: Audio buffer with shape (num_samples, 2) and dtype float32

**Raises:**
- `RuntimeError`: If rendering fails
- `ValueError`: If preset is invalid

#### assess_quality()

Assess audio quality using the C++ MOO optimizer.

```python
quality = engine.assess_quality(
    audio=audio_buffer,  # NumPy array (num_samples, 2)
    role="pad",
    context={"tempo": 120.0, "key": 0, "scale": "major"}
)
```

**Parameters:**
- `audio` (np.ndarray): Audio buffer, shape (num_samples, 2), dtype float32
- `role` (str): Preset role (pad, bass, lead, drum, fx)
- `context` (Dict[str, Any], optional): Musical context

**Returns:**
- `Dict[str, Any]`: Quality metrics
  ```python
  {
      "overall_score": 0.85,
      "semantic_match": 0.82,
      "mix_readiness": 0.88,
      "perceptual_quality": 0.87,
      "stability": 0.95,
      "violations": [],
      "warnings": ["Some warning"]
  }
  ```

#### get_status()

Get system status.

```python
status = engine.get_status()
```

**Returns:**
- `Dict[str, Any]`: Status dictionary
  ```python
  {
      "initialized": True,
      "loaded_presets": 10,
      "cpu_usage": 0.45,
      "memory_usage": 256.0,
      "active_features": ["moo", "dsp_ir", "semantic"],
      "engine": "cpp"
  }
  ```

#### Other Methods

```python
# Load preset from file
engine.load_preset("/path/to/preset.json")

# Get available presets
presets = engine.get_available_presets()  # Returns List[str]

# Set configuration
engine.set_configuration({
    "semantic_model": "advanced",
    "quality_threshold": "0.8"
})
```

### Convenience Functions

Module-level functions for quick access:

```python
from cpp_engine import render_audio, assess_quality, get_engine

# Render audio
audio = render_audio(preset, context, duration=2.0)

# Assess quality
quality = assess_quality(audio, "pad", context)

# Get engine instance
engine = get_engine()
```

## C++ API (pybind11 Module)

### CPPAudioEngine

Low-level C++ engine exposed to Python.

```python
import aiaudio_cpp

engine = aiaudio_cpp.CPPAudioEngine()
```

#### Methods

##### render_audio()

```python
audio = engine.render_audio(
    preset_dict={
        "prompt": "Warm pad",
        "role": "pad",
        "constraints": {
            "max_cpu": 0.8,
            "max_latency": 10.0,
            "lufs_target": -18.0,
            "true_peak_limit": -1.0
        }
    },
    context_dict={
        "tempo": 120.0,
        "key": 0,
        "scale": "major",
        "time_signature": [4, 4]
    },
    duration=2.0
)
```

##### assess_quality()

```python
quality = engine.assess_quality(
    audio=audio_buffer,
    role="pad",
    context_dict={"tempo": 120.0, "key": 0, "scale": "major"}
)
```

##### get_status()

```python
status = engine.get_status()
```

##### load_preset()

```python
engine.load_preset("/path/to/preset.json")
```

##### get_available_presets()

```python
presets = engine.get_available_presets()
```

##### set_configuration()

```python
engine.set_configuration({"key": "value"})
```

### Enums

#### Role

```python
from aiaudio_cpp import Role

Role.UNKNOWN
Role.PAD
Role.BASS
Role.LEAD
Role.DRUM
Role.FX
```

### Module Functions

```python
# Get version
version = aiaudio_cpp.get_version()  # "1.0.0"

# Get sample rate
sr = aiaudio_cpp.get_sample_rate()  # 44100
```

## Data Format Specifications

### Audio Buffer Format

Audio buffers are represented as NumPy arrays:

```python
audio: np.ndarray
# Shape: (num_samples, 2)  # Stereo
# dtype: np.float32
# Range: [-1.0, 1.0]
# Sample rate: 44100 Hz (default)
```

Example:
```python
# 2 seconds of stereo audio at 44.1 kHz
num_samples = 2 * 44100
audio = np.zeros((num_samples, 2), dtype=np.float32)
```

### Preset Dictionary Format

```python
preset = {
    "name": "Warm Pad",
    "role": "pad",  # pad, bass, lead, drum, fx
    "description": "Warm atmospheric pad with lush reverb",
    "category": "pad",
    "parameters": {
        "frequency": 440.0,
        "amplitude": 0.5,
        "attack": 0.2,
        "decay": 0.5,
        "sustain": 0.7,
        "release": 2.0
    },
    "constraints": {
        "max_cpu": 0.8,
        "max_latency": 10.0,
        "lufs_target": -18.0,
        "true_peak_limit": -1.0
    }
}
```

### Context Dictionary Format

```python
context = {
    "tempo": 120.0,           # BPM
    "key": 0,                 # 0=C, 1=C#, ..., 11=B
    "scale": "major",         # "major" or "minor"
    "time_signature": [4, 4]  # [numerator, denominator]
}
```

## Threading and GIL Management

The bridge automatically manages the Python Global Interpreter Lock (GIL) for optimal performance:

```python
# GIL is released during C++ processing
audio = engine.render_audio(preset, context)
# GIL is reacquired before returning to Python
```

This allows:
- Concurrent Python threads while C++ is processing
- Maximum C++ performance without GIL overhead
- Safe multithreading

## Error Handling

### Python Exceptions

```python
try:
    audio = engine.render_audio(preset, context)
except ValueError as e:
    # Invalid preset parameters
    print(f"Invalid preset: {e}")
except RuntimeError as e:
    # C++ rendering error
    print(f"Rendering failed: {e}")
except Exception as e:
    # Unexpected error
    print(f"Error: {e}")
```

### C++ Exceptions

C++ exceptions are automatically converted to Python exceptions:

```cpp
// C++ code
throw std::runtime_error("DSP error");
// Becomes Python RuntimeError
```

## Performance Considerations

### Zero-Copy Buffer Transfer

The bridge uses NumPy's buffer protocol for efficient memory transfer:

```python
# No copying - direct memory access
audio = engine.render_audio(preset, context)  # O(1) copy
```

### Buffer Reuse

For repeated rendering, reuse buffers:

```python
# Preallocate buffer
num_samples = int(2.0 * 44100)
audio_buffer = np.zeros((num_samples, 2), dtype=np.float32)

# Render into existing buffer (if supported)
engine.render_audio(preset, context, duration=2.0)
```

### Latency Optimization

```python
# Reduce duration for faster previews
audio = engine.render_audio(preset, context, duration=1.0)

# Use caching for frequently accessed presets
engine.load_preset("frequently_used.json")
```

## Integration with Flask Web Server

```python
from flask import Flask, jsonify, request
from cpp_engine import get_engine
import numpy as np

app = Flask(__name__)
engine = get_engine()

@app.route('/api/render', methods=['POST'])
def render():
    data = request.json
    preset = data['preset']
    context = data.get('context', {})
    duration = data.get('duration', 2.0)
    
    try:
        audio = engine.render_audio(preset, context, duration)
        
        # Convert to list for JSON serialization
        audio_list = audio.tolist()
        
        return jsonify({
            'audio': audio_list,
            'sample_rate': 44100,
            'channels': 2,
            'duration': duration
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 400
```

## Testing

### Unit Tests

```python
import unittest
import numpy as np
from cpp_engine import CPPAudioEngineAdapter

class TestCPPBridge(unittest.TestCase):
    def setUp(self):
        self.engine = CPPAudioEngineAdapter(fallback_to_python=True)
    
    def test_render_audio(self):
        preset = {
            "name": "Test",
            "role": "pad",
            "description": "Test preset",
            "category": "pad"
        }
        context = {"tempo": 120.0, "key": 0, "scale": "major"}
        
        audio = self.engine.render_audio(preset, context, duration=1.0)
        
        self.assertEqual(audio.shape, (44100, 2))
        self.assertEqual(audio.dtype, np.float32)
        self.assertTrue(np.all(np.abs(audio) <= 1.0))
    
    def test_assess_quality(self):
        audio = np.random.randn(44100, 2).astype(np.float32) * 0.1
        quality = self.engine.assess_quality(audio, "pad", {})
        
        self.assertIn("overall_score", quality)
        self.assertGreater(quality["overall_score"], 0.0)
        self.assertLessEqual(quality["overall_score"], 1.0)
```

## Troubleshooting

### Module Not Found

```bash
# Check if module is built
ls build/aiaudio_cpp*.so

# Add to Python path
export PYTHONPATH=/path/to/build:$PYTHONPATH

# Or install
cd build && make install
```

### Import Error

```python
# Check if C++ engine is available
import aiaudio_cpp
print(aiaudio_cpp.get_version())
```

### Linking Errors

```bash
# Check dependencies
ldd build/aiaudio_cpp*.so

# Install missing libraries
sudo apt-get install libyaml-cpp-dev libjsoncpp-dev
```

## Future Enhancements

- [ ] Streaming audio API for real-time processing
- [ ] Batch rendering for multiple presets
- [ ] GPU acceleration support
- [ ] Advanced buffer management with memory pools
- [ ] Async rendering with callbacks
