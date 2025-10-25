# One-Click Sound Pipeline: Unified Architecture Document

## Executive Summary

This document defines the architecture for unifying the Python-based preset pipeline (`AudioPresetGenerator`) with the C++-based audio rendering engine (`AIAudioGenerator`). The goal is to create a seamless, low-latency "one-click sound" experience where clicking a preset tile in the web interface immediately produces a high-quality audio preview.

**Version:** 1.0  
**Date:** 2025-10-25  
**Status:** Design Phase - Awaiting Team Approval

---

## 1. User Stories & Experience Goals

### 1.1 Primary User Story
> **As a music producer**, I want to click on a sound preset tile and immediately hear a high-quality audio preview, so I can quickly audition sounds and make creative decisions.

### 1.2 Key User Scenarios

#### Scenario A: Browse and Audition
1. User opens the web interface
2. User sees a grid of categorized presets (pads, bass, leads, etc.)
3. User clicks a preset tile
4. Audio preview plays within 100ms
5. User can stop the preview or let it complete
6. User can audition multiple presets sequentially or simultaneously

#### Scenario B: Search and Filter
1. User searches for "warm pad"
2. System returns semantically relevant presets
3. User filters by role (PAD, BASS, LEAD)
4. User clicks a preset to preview
5. System displays quality metrics and parameters

#### Scenario C: Desktop Pro-Audio Workflow
1. Producer opens desktop application (PySide/Electron)
2. Application connects to local C++ engine
3. Presets load with full metadata
4. Producer auditions sounds offline with sub-5ms latency
5. System displays CPU usage, buffer underruns, and quality scores

### 1.3 Success Metrics
- **Latency**: Click-to-sound < 100ms (web), < 50ms (desktop)
- **Quality**: MOO quality score > 0.75 for all presets
- **Stability**: Zero audio glitches or dropouts during playback
- **Compatibility**: Works on MSYS2/Windows, Linux, and macOS
- **User Satisfaction**: > 90% of presets play successfully on first click

---

## 2. System Architecture Overview

### 2.1 High-Level Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                     USER INTERFACE                          │
│  (React/Vite Web App or PySide Desktop App)                 │
│  - Preset Grid Display                                      │
│  - Search & Filter Controls                                 │
│  - Real-time Playback Status                                │
└────────────────┬────────────────────────────────────────────┘
                 │ HTTP REST API / Local IPC
                 ▼
┌─────────────────────────────────────────────────────────────┐
│              PYTHON ORCHESTRATION LAYER                     │
│  (Flask/FastAPI Web Server + AudioInterface)                │
│  - Preset Loading & Parsing                                 │
│  - Semantic Search & Filtering                              │
│  - Policy Application                                       │
│  - Normalization                                            │
└────────────────┬────────────────────────────────────────────┘
                 │ Python-C++ Bridge (pybind11)
                 ▼
┌─────────────────────────────────────────────────────────────┐
│              C++ AUDIO RENDERING ENGINE                     │
│  (AIAudioGenerator + DSPGraph)                              │
│  - DSP Graph Construction                                   │
│  - Real-time Audio Rendering                                │
│  - MOO Quality Assessment                                   │
│  - Audio Safety & Headroom Management                       │
└────────────────┬────────────────────────────────────────────┘
                 │ Audio Buffers (float32)
                 ▼
┌─────────────────────────────────────────────────────────────┐
│              AUDIO OUTPUT LAYER                             │
│  (sounddevice/pygame/Web Audio API)                         │
│  - Audio Buffer Playback                                    │
│  - Device Management                                        │
│  - Latency Compensation                                     │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Component Responsibilities

#### Python Layer Responsibilities
- **Preset Management**: Load and parse JSON presets from `electronic_track.json`, `guitar.json`, etc.
- **Semantic Processing**: Extract embeddings, apply semantic search, match user queries
- **Normalization**: Apply `PresetNormalizer` to ensure perceptual consistency
- **Policy Application**: Enforce role-based constraints from `config/roles.yaml`
- **Web API**: Serve REST endpoints (`/api/presets`, `/api/render`)
- **UI State Management**: Track playing presets, update UI in real-time

#### C++ Layer Responsibilities
- **DSP Graph Construction**: Build `DSPGraph` from normalized preset data
- **Audio Rendering**: Execute real-time audio synthesis with low latency
- **MOO Optimization**: Evaluate quality metrics (semantic match, mix readiness, perceptual quality, stability)
- **Safety Systems**: Prevent clipping, manage headroom, detect instabilities
- **Trace Generation**: Provide full reproducibility and debugging information

#### Bridge Layer Responsibilities (pybind11)
- **Data Translation**: Convert Python dictionaries to C++ `GenerationRequest` structs
- **Buffer Management**: Efficiently transfer audio buffers (NumPy ↔ std::vector)
- **Error Handling**: Propagate exceptions between Python and C++
- **Thread Safety**: Ensure safe concurrent access to C++ engine

---

## 3. API Contract Specification

### 3.1 Python → C++ Bridge API

#### Function: `render_audio`
```python
def render_audio(preset_dict: Dict[str, Any], 
                 context: Dict[str, Any],
                 duration: float = 2.0) -> np.ndarray:
    """
    Render audio from a preset using the C++ engine.
    
    Args:
        preset_dict: Normalized preset parameters
            {
                "role": "pad",  # pad, bass, lead, drum, fx
                "stages": [
                    {
                        "type": "oscillator",
                        "frequency": 440.0,
                        "amplitude": 0.5,
                        "waveform": "sine"
                    },
                    {
                        "type": "envelope",
                        "attack": 0.2,
                        "decay": 0.5,
                        "sustain": 0.7,
                        "release": 2.0
                    }
                ],
                "connections": [
                    {"from": "osc1", "to": "env1", "weight": 1.0}
                ]
            }
        context: Musical context
            {
                "tempo": 120.0,
                "key": 0,  # 0 = C, 1 = C#, etc.
                "scale": "major",
                "time_signature": [4, 4]
            }
        duration: Audio duration in seconds
        
    Returns:
        np.ndarray: Audio buffer, shape (num_samples, 2), dtype float32
        
    Raises:
        RuntimeError: If rendering fails
        ValueError: If preset is invalid
    """
```

#### Function: `assess_quality`
```python
def assess_quality(audio: np.ndarray, 
                   role: str,
                   context: Dict[str, Any]) -> Dict[str, Any]:
    """
    Assess audio quality using MOO optimizer.
    
    Args:
        audio: Audio buffer (num_samples, 2), float32
        role: Preset role (pad, bass, lead, drum, fx)
        context: Musical context
        
    Returns:
        {
            "overall_score": 0.85,
            "semantic_match": 0.82,
            "mix_readiness": 0.88,
            "perceptual_quality": 0.87,
            "stability": 0.95,
            "violations": [],
            "warnings": ["Some headroom warning"]
        }
    """
```

### 3.2 Data Schema Alignment

#### Shared Preset Schema (JSON)
```json
{
  "name": "Warm Pad",
  "role": "pad",
  "description": "Warm atmospheric pad with lush reverb",
  "tags": ["warm", "atmospheric", "reverb"],
  "version": "1.0",
  "metadata": {
    "author": "AI Audio Generator",
    "created": "2025-10-25T00:00:00Z",
    "quality_score": 0.85
  },
  "stages": [
    {
      "id": "osc1",
      "type": "oscillator",
      "parameters": {
        "frequency": {"value": 440.0, "unit": "Hz"},
        "amplitude": {"value": 0.5, "unit": "linear"},
        "waveform": "sine"
      }
    },
    {
      "id": "env1",
      "type": "envelope",
      "parameters": {
        "attack": {"value": 0.2, "unit": "s"},
        "decay": {"value": 0.5, "unit": "s"},
        "sustain": {"value": 0.7, "unit": "linear"},
        "release": {"value": 2.0, "unit": "s"}
      }
    }
  ],
  "connections": [
    {
      "from": "osc1",
      "to": "env1",
      "weight": 1.0
    }
  ]
}
```

### 3.3 Buffer Format Specification
- **Sample Rate**: 44100 Hz (configurable)
- **Bit Depth**: 32-bit float
- **Channels**: 2 (stereo)
- **Interleaving**: Non-interleaved (planar) for efficiency
- **Endianness**: Native (platform-specific)
- **Buffer Size**: 1024-8192 samples (configurable)

### 3.4 Threading Model
- **Python Main Thread**: Flask/FastAPI web server, UI event handling
- **Python Worker Threads**: Audio playback (sounddevice), async I/O
- **C++ Audio Thread**: Real-time DSP processing (pinned to CPU core)
- **GIL Management**: Release GIL during C++ rendering to avoid blocking
- **Synchronization**: Lock-free audio buffers where possible

---

## 4. Latency Optimization Strategy

### 4.1 Latency Budget Breakdown
| Component | Target (ms) | Notes |
|-----------|-------------|-------|
| UI Click Event | 5 | Negligible |
| HTTP Request (Local) | 10 | Flask/FastAPI overhead |
| Preset Loading | 20 | JSON parsing + normalization |
| Python→C++ Bridge | 10 | pybind11 overhead |
| DSP Graph Construction | 30 | One-time cost |
| Audio Rendering (First Buffer) | 20 | Real-time synthesis |
| Audio Playback Start | 5 | sounddevice latency |
| **Total** | **100 ms** | Click-to-sound target |

### 4.2 Optimization Techniques
1. **Preset Caching**: Pre-parse and cache normalized presets on startup
2. **DSP Graph Pooling**: Reuse graph structures for similar presets
3. **Async Rendering**: Start rendering first buffer while constructing full graph
4. **JIT Compilation**: Use template specialization for hot paths
5. **SIMD Optimization**: Vectorize oscillator and filter operations
6. **Zero-Copy Buffers**: Share memory between Python and C++ where possible

---

## 5. Quality Assurance & Safety

### 5.1 Audio Safety Checks
- **Clipping Detection**: Monitor for samples exceeding ±1.0
- **Headroom Management**: Maintain -6dB headroom for mixing
- **DC Offset Detection**: Filter out DC components
- **Instability Detection**: Check for runaway feedback or NaN values
- **LUFS Target**: Normalize to -18 LUFS for consistency

### 5.2 Error Handling
```python
try:
    audio = cpp_engine.render_audio(preset, context, duration=2.0)
except ValueError as e:
    # Invalid preset parameters
    logger.error(f"Invalid preset: {e}")
    return {"error": "Invalid preset configuration"}
except RuntimeError as e:
    # Rendering failure (C++ exception)
    logger.error(f"Rendering failed: {e}")
    return {"error": "Audio rendering failed"}
except Exception as e:
    # Unexpected error
    logger.exception("Unexpected error during rendering")
    return {"error": "Internal server error"}
```

### 5.3 Quality Validation
- **Pre-Render Validation**: Check preset schema against JSON Schema
- **Post-Render Validation**: Run MOO quality assessment
- **Threshold Gating**: Reject audio with quality score < 0.5
- **User Feedback Loop**: Log failed renders for analysis

---

## 6. MSYS2/Windows Compatibility

### 6.1 Platform-Specific Considerations
- **Path Separators**: Use `std::filesystem::path` for cross-platform paths
- **Dynamic Libraries**: Use `.dll` (Windows), `.so` (Linux), `.dylib` (macOS)
- **Threading**: Use `std::thread` instead of platform-specific APIs
- **Audio APIs**: Use `sounddevice` (cross-platform) or native APIs via abstraction

### 6.2 MSYS2 Build Requirements
```bash
# Install MSYS2 packages
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-yaml-cpp
pacman -S mingw-w64-x86_64-jsoncpp
pacman -S mingw-w64-x86_64-python
pacman -S mingw-w64-x86_64-pybind11

# Build with MinGW
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make -j$(nproc)
```

### 6.3 CMake Platform Guards
```cmake
if(WIN32)
    # Windows-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32_LEAN_AND_MEAN")
elseif(UNIX AND NOT APPLE)
    # Linux-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif(APPLE)
    # macOS-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework CoreAudio")
endif()
```

---

## 7. Implementation Roadmap

### Phase 1: Foundation (Week 1)
- [ ] Create Python-C++ bridge with pybind11
- [ ] Implement basic `render_audio()` function
- [ ] Add NumPy buffer conversion
- [ ] Test with simple sine wave preset

### Phase 2: Integration (Week 2)
- [ ] Integrate with `AudioInterface` class
- [ ] Add REST endpoint `/api/render`
- [ ] Implement preset caching
- [ ] Test with all JSON presets

### Phase 3: Optimization (Week 3)
- [ ] Profile latency bottlenecks
- [ ] Optimize DSP graph construction
- [ ] Add async rendering
- [ ] Achieve < 100ms click-to-sound

### Phase 4: Quality & Safety (Week 4)
- [ ] Implement MOO quality assessment
- [ ] Add audio safety checks
- [ ] Validate against all test presets
- [ ] Fix quality violations

### Phase 5: UI/UX (Week 5)
- [ ] Build React/Vite web interface
- [ ] Add real-time playback indicators
- [ ] Implement search and filter
- [ ] Add quality metrics display

### Phase 6: Production Hardening (Week 6)
- [ ] MSYS2/Windows testing
- [ ] CI/CD pipeline setup
- [ ] Performance benchmarking
- [ ] Documentation and tutorials

---

## 8. Success Criteria

### 8.1 Functional Requirements
- ✅ Single `make all` command builds entire system
- ✅ Web interface loads and displays all presets
- ✅ Clicking a preset plays audio within 100ms
- ✅ Audio quality score > 0.75 for all presets
- ✅ No audio glitches or dropouts
- ✅ Works on MSYS2/Windows, Linux, and macOS

### 8.2 Non-Functional Requirements
- **Performance**: Render 8-second audio in < 50ms
- **Reliability**: 99.9% successful render rate
- **Scalability**: Support 1000+ presets without degradation
- **Usability**: Intuitive UI with < 5-minute learning curve
- **Maintainability**: Clean separation of concerns, comprehensive tests

### 8.3 Acceptance Criteria
1. **Build System**: `make all` succeeds on all platforms
2. **Latency Test**: 95% of presets play within 100ms
3. **Quality Test**: All presets pass MOO validation
4. **Stress Test**: System handles 100 concurrent requests
5. **User Test**: 3 external users successfully audition presets

---

## 9. Risk Assessment & Mitigation

### 9.1 Technical Risks
| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Python-C++ bridge overhead | Medium | High | Use zero-copy buffers, profile extensively |
| MSYS2 build failures | High | High | Add platform guards, test on CI |
| Audio glitches | Medium | High | Implement safety checks, test thoroughly |
| High latency | Medium | Medium | Optimize hot paths, use async rendering |
| Preset incompatibility | Low | Medium | Validate schema, provide migration tool |

### 9.2 Schedule Risks
- **Dependency**: pybind11 learning curve → Allocate 2 days for research
- **Blocker**: MSYS2 toolchain issues → Maintain Linux fallback
- **Delay**: Performance optimization → Start early, iterate continuously

---

## 10. Team Sign-Off

**Approval Required From:**
- [ ] Engineering Lead: _____________________
- [ ] Product Manager: _____________________
- [ ] QA Lead: _____________________
- [ ] DevOps Lead: _____________________

**Date Approved:** _____________________

---

## 11. References

- [pybind11 Documentation](https://pybind11.readthedocs.io/)
- [NumPy C API](https://numpy.org/doc/stable/reference/c-api/)
- [sounddevice Documentation](https://python-sounddevice.readthedocs.io/)
- [JSON Schema Specification](https://json-schema.org/)
- [Audio Programming Best Practices](https://github.com/spotify/audio-research)

---

**Document Version Control:**
- v1.0 (2025-10-25): Initial draft
