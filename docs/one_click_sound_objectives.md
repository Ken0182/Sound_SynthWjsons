# One-Click Sound: Preset-to-Playback Objectives and Architecture

## Objectives
- Unify Python "AudioPresetGenerator" semantic pipeline with C++ "AIAudioGenerator" renderer.
- Enable a one-click flow: user clicks a preset tile → instant audio preview.
- Provide a single build entry (make all) and MSYS2 compatibility for Windows.
- Ensure synchronized outputs (metadata, quality metrics) and low-latency previews (< 10 ms buffer, < 150 ms click-to-sound).

## User stories
- As a user, I click a preset tile in the grid and immediately hear a short preview.
- As a sound designer, I search by text (e.g., "lush pad") and get relevant presets with roles and explanations.
- As a QA engineer, I run make all on Linux and MSYS2/Windows and obtain a working web UI and CLI demo.
- As a developer, I can run a Python script to generate a normalized preset JSON and ask C++ to render it to a buffer.

## End-to-end flow
1) UI event
- Click preset tile or text search → POST /api/render with decision context.

2) Python semantic engine
- Parse: `core_models.JsonPresetParser` loads the selected preset (JSON).
- Normalize: `normalizer.UnitNormalizer` converts units, validates constraints.
- Search/decision: `semantic_embeddings.SemanticSearchEngine` and `decision_heads.DecisionEngine` produce decisions and a DSP graph summary.
- Compile request: Build an `AIAudioGenerator::GenerationRequest`-equivalent dict (prompt, role, context, constraints).

3) Python↔C++ bridge (pybind11)
- Adapter `cpp_engine.py` marshals dict → C++ structs and calls `AIAudioGenerator.generate`.
- Returns: float32 NumPy array (mono or stereo), plus metadata: quality score, warnings, explanation, trace.

4) C++ rendering
- C++ `AIAudioGenerator` applies semantic search (optional), decisions, policies, and renders via `DSPGraph`.
- Produces `GenerationResult { audio, trace, qualityScore, warnings, explanation }`.

5) Playback / Telemetry
- Web client receives a preview URL or audio buffer reference. Desktop path uses `audio_interface.AudioInterface` to play NumPy audio directly.
- Metrics: latency, underruns, and quality_score logged; status exposed via `/api/status`.

## Responsibilities
- Python
  - Preset parsing/normalization, semantic search, policy application.
  - Build decision context and prepare C++ request.
  - Serve REST API endpoints and orchestrate preview playback.
- C++
  - Strongly typed IR, safety checks, decision heads application, real-time rendering.
  - Provide diagnostics (quality score, warnings) and buffer output.

## Interfaces and Contracts
- Input: JSON preset (see `config/preset_schema.yaml`).
- Bridge API: `render(request_dict) -> (np.ndarray float32, metadata)`.
- Buffer: interleaved float32 stereo or planar mono; sample rate 44100.
- Threads: C++ render may use worker threads; Python adapter offers sync call and optional async preview token.
- Lifetime: Returned NumPy owns a copy; previews are finite-length (e.g., 1–3 s) and cancelable.

## Success Criteria
- Single `make all` builds C++ core, Python deps, web UI on Linux and MSYS2.
- `/api/presets` and `/api/render` return data consistently across OSes.
- Click-to-sound p50 ≤ 150 ms, buffer size ≤ 512 samples, no audible underruns.
- Quality telemetry and warnings available in UI and logs.

## MSYS2 Notes
- Use `CMakePresets.json` with `MinGW Makefiles` and `-DMSYS=ON`.
- Avoid `pthread` on Windows; use conditional linking.
- Ensure `pybind11` and NumPy are discoverable via `Python3` and `pybind11` CMake packages.

## Approval Process
- This document and the bridge contract (`docs/api/python_cpp_bridge.md`) must be approved by the team before code changes that alter request/response shapes or threading semantics.