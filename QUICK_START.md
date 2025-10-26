# Quick Start Guide - AI Audio Generator

## One-Minute Setup

```bash
# 1. Install dependencies (platform-specific)
make install-system-deps

# 2. Build everything
make all

# 3. Start web server
python web_server.py
```

Open browser: http://localhost:8080

---

## Platform-Specific Commands

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y cmake g++ python3 python3-pip \
                       libyaml-cpp-dev libjsoncpp-dev pybind11-dev
pip3 install -r requirements.txt
make all
python3 web_server.py
```

### macOS
```bash
brew install cmake yaml-cpp jsoncpp python pybind11
pip3 install -r requirements.txt
make all
python3 web_server.py
```

### MSYS2/Windows
```bash
# In MSYS2 MINGW64 terminal:
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-yaml-cpp mingw-w64-x86_64-jsoncpp \
          mingw-w64-x86_64-python mingw-w64-x86_64-pybind11
pip install -r requirements.txt
make all
python web_server.py
```

---

## Common Tasks

### Build
```bash
make all              # Build everything
make build-cpp        # Build C++ only
make build-python     # Install Python deps
```

### Run
```bash
make serve            # Start web server
make dev-cpp          # Run C++ demo
make audio-demo       # Audio demonstration
```

### Test
```bash
make test             # Run all tests
make validate-presets # Validate JSON presets
```

### Clean
```bash
make clean            # Clean all artifacts
make clean-cpp        # Clean C++ build only
```

---

## Verify Installation

```bash
# Check C++ build
ls build/src/aiaudio_generator

# Check Python module
python -c "import aiaudio_cpp; print(aiaudio_cpp.get_version())"

# Check presets
python audio_interface.py --load-presets

# Test web API
curl http://localhost:8080/api/presets
```

---

## Troubleshooting

### "pybind11 not found"
```bash
pip install pybind11
# Or: sudo apt-get install pybind11-dev (Linux)
# Or: brew install pybind11 (macOS)
# Or: pacman -S mingw-w64-x86_64-pybind11 (MSYS2)
```

### "Module aiaudio_cpp not found"
```bash
export PYTHONPATH=$(pwd)/build:$PYTHONPATH
# Or rebuild: make clean && make all
```

### "cmake version too old"
```bash
# Upgrade CMake to 3.20+
pip install --upgrade cmake
```

---

## File Structure

```
ai-audio-generator/
├── docs/
│   ├── one_click_sound_objectives.md    # Architecture
│   └── api/python_cpp_bridge.md         # API docs
├── config/
│   ├── preset_schema.yaml               # Data schema
│   └── quality_weights.json             # Quality config
├── src/
│   ├── python_bindings.cpp              # pybind11 bindings
│   └── [other C++ files]
├── cpp_engine.py                        # Python adapter
├── web_server.py                        # Flask server
├── schema_validator.py                  # Validation tool
├── CMakeLists.txt                       # Build config
├── CMakePresets.json                    # Platform presets
├── Makefile                             # Build automation
└── web/                                 # React/Vite UI
    ├── package.json
    └── vite.config.js
```

---

## Next Steps

1. **Read Documentation:**
   - `docs/one_click_sound_objectives.md` - System architecture
   - `docs/api/python_cpp_bridge.md` - API reference

2. **Explore Code:**
   - `cpp_engine.py` - Python-C++ integration
   - `src/python_bindings.cpp` - C++ bindings
   - `web_server.py` - REST API

3. **Customize:**
   - Edit `config/preset_schema.yaml` for custom stages
   - Modify `config/quality_weights.json` for quality tuning
   - Add presets in JSON format

4. **Deploy:**
   - See `INTEGRATION_COMPLETE.md` for deployment guide
   - Use Docker for containerization (future work)
   - Set up CI/CD pipeline (future work)

---

## Help & Support

```bash
make help             # Show all make targets
python web_server.py --help
python audio_interface.py --help
python schema_validator.py --help
```

**Documentation:**
- Architecture: `docs/one_click_sound_objectives.md`
- API Reference: `docs/api/python_cpp_bridge.md`
- Integration Summary: `INTEGRATION_COMPLETE.md`

**Common Make Targets:**
- `make all` - Build everything
- `make serve` - Start server
- `make test` - Run tests
- `make clean` - Clean build
- `make help` - Show all targets
