## Python↔C++ Bridge (pybind11) — Contract and Usage

### Overview
The C++ core (`aiaudio::AIAudioGenerator`) is exposed to Python via an optional pybind11 module named `aiaudio_python`. Python code passes a request dictionary and receives a NumPy float32 audio buffer with diagnostics.

### Threading & lifetime
- Preview calls are synchronous by default (`render_sync`); they block until the buffer is rendered.
- Engine lifetime: use `aiaudio_python.Engine()` for repeated calls to avoid reinitialization overhead.
- Async previews: planned via handles in a future update; lifetime rules will guarantee cancel-safe operations.

### Buffer format
- NumPy `float32` mono buffer by default; sample rate 44.1 kHz in the current engine. Stereo support can be added by returning shape `(N, 2)` interleaved.

### Request schema (Python dict)
```
{
  "prompt": "warm analog pad",
  "role": "PAD",                 // or enum index
  "context": { "tempo": 120.0, "key": 0, "scale": "major" },
  "constraints": {
    "maxCPU": 0.8,
    "maxLatency": 10.0,
    "noHardClips": true,
    "truePeakLimit": -1.0,
    "lufsTarget": -18.0
  },
  "useSemanticSearch": true,
  "applyPolicies": true,
  "optimizeForMOO": true
}
```

### Python usage
```python
from cpp_engine import is_available, render_sync

if is_available():
    audio, meta = render_sync("dreamy pad", role="PAD", tempo=120.0)
    # audio: np.ndarray(float32), meta: dict with quality and warnings
```

### CMake integration (optional)
- The build enables bindings only if `pybind11` is found.
- Target: `aiaudio_python` (pybind11 module) links against `aiaudio_core`.
- MSYS2 supported via `CMakePresets.json` (MinGW Makefiles) and `Threads::Threads`.
