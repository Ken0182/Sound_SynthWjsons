## One‑Click Sound — End‑to‑End Objectives and Architecture

### Vision
A single click on a preset tile produces an immediate, low‑latency sound preview. The flow unifies the Python semantic pipeline with the C++ rendering engine and runs consistently on Linux and MSYS2/Windows.

### User stories
- "As a producer, I click a preset tile and hear a 1–2s preview within 100 ms UI latency and <15 ms audio render latency for first buffer."
- "As a sound designer, I search by text (e.g., “warm analog pad”), see ranked presets, and audition the top result instantly."
- "As an engineer, I can run one make all that builds the C++ engine, optional Python bindings, and the web UI scaffold on Linux and MSYS2."

### Unified flow (preset tile → Python semantics → C++ render)
1) UI action: User clicks a preset tile or submits a text query.
2) Python orchestration (AudioPresetGenerator):
   - Parse/normalize preset to canonical units (seconds/Hz/ratios).
   - Run semantic search and role policies; produce a decision context.
   - Validate against `config/preset_schema.yaml` and compute quality weights from `config/quality_weights.json`.
3) Cross‑stack bridge:
   - Python builds a `GenerationRequest` dict (prompt/role/context/constraints).
   - Send to C++ via pybind11 (`aiaudio_python.Engine.render_sync`) or fall back to Python preview.
4) C++ core (AIAudioGenerator):
   - Create graph from prompt, apply decision heads/policies, render an audio buffer (float32), collect diagnostics (quality, trace, warnings).
5) Playback & metrics:
   - Return NumPy float32 buffer (interleaved if stereo) and metadata to Python.
   - UI plays audio and displays latency, CPU usage, and underrun warnings.

### Shared responsibilities
- Python (semantic, UX, I/O):
  - Preset parsing/normalization; semantic search; role/policy application.
  - Schema validation; request assembly; initiating render; REST/Web UI.
  - Optional offline preview (pure‑Python tone synthesis) when C++ is unavailable.
- C++ (performance, correctness):
  - Typed DSP IR, constraints, safety checks; deterministic render.
  - Request/response structs, quality assessment, and trace diagnostics.
  - Real‑time friendliness (threading, buffer sizes, minimal allocations).

### Success criteria
- Build:
  - A single `make all` (or `cmake --preset <name> && cmake --build --preset <name>`) builds engine, optional bindings, and web scaffold on Linux and MSYS2.
- Output sync:
  - Python and C++ agree on schema, units, and defaults; identical requests yield comparable results and diagnostics.
- Latency:
  - UI interaction latency ≤ 100 ms.
  - First audio buffer render ≤ 15 ms for a 512‑sample buffer @ 48 kHz on reference hardware.
  - No hard clipping by default; LUFS target and true‑peak respected.

### UI/UX goals
- Fast grid of preset tiles with immediate visual feedback and clear play/stop state.
- Text search and category filters; simple metrics surface (quality score, warnings).
- Accessibility: keyboard navigation; reduced motion; high‑contrast and dark mode.

### Threading, buffers, and formats (contract excerpt)
- Buffer format: float32, 1‑D mono or 2‑D (N, 2) interleaved stereo; default sample rate 44.1 kHz or 48 kHz.
- Python→C++ calls are synchronous for previews, with optional async handles for continuous playback.
- Lifetime: engine instance owns internal resources; async previews must return handles that can be cancelled.

### MSYS2/Windows compatibility notes
- Prefer `Threads::Threads` over `pthread` and guard `-lm`/`-march=native` usage.
- Provide `CMakePresets.json` with a `MinGW Makefiles` preset.
- Avoid Linux‑specific Makefile commands in the default path; delegate to CMake.

### Metrics & observability
- Log render time, CPU usage, memory, underruns; expose via `/api/status` and `/api/render`.
- Persist quality weights in `config/quality_weights.json` and include in diagnostics.

### Approval & change management
- This document is the basis for implementation. Obtain team approval and sign‑off before altering public APIs or behavior that affects UI latency or schema compatibility.
