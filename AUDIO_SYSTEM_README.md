# AI Audio Generator - Complete Audio System

A comprehensive audio system that converts JSON sound presets into playable audio with a modern web interface.

## Features

- **JSON Sound Preset Loading**: Loads sound presets from multiple JSON datasets
- **Real-time Audio Playback**: Play sounds instantly with start/stop controls
- **Modern Web Interface**: Responsive grid-based UI with hover effects and keyboard support
- **Multiple Audio Engines**: Support for pygame, sounddevice, and mock audio
- **REST API**: Complete API for programmatic control
- **Cross-platform**: Works on Linux, macOS, and Windows

## Quick Start

### 1. Install Dependencies

```bash
# Install system dependencies
make install-deps

# Or install manually
sudo apt-get update
sudo apt-get install -y cmake g++ python3 python3-pip nodejs npm
sudo apt-get install -y libopencv-dev libyaml-cpp-dev nlohmann-json3-dev
sudo apt-get install -y libgtest-dev libgmock-dev pkg-config

# Install Python dependencies
pip install -r requirements.txt
```

### 2. Build the System

```bash
# Build everything
make all

# Or build components individually
make build-cpp      # Build C++ audio engine
make build-python   # Install Python dependencies
make build-web      # Build web interface
```

### 3. Run the Demo

```bash
# Run the interactive demo
python demo.py

# Or start the web server directly
make serve-dev
```

The web interface will be available at `http://localhost:8080`

## Usage

### Web Interface

1. **Browse Presets**: View all available sound presets in a responsive grid
2. **Search & Filter**: Use the search box and category filter to find specific sounds
3. **Play Sounds**: Click any preset tile to play the sound
4. **Stop Sounds**: Click again to stop, or use the "Stop All" button
5. **View Details**: Click the info button to see preset parameters
6. **Keyboard Support**: Use Escape key to stop all sounds

### Command Line Interface

```bash
# Load and list all presets
python audio_interface.py --load-presets

# Run audio demo
python audio_interface.py --demo

# Use specific audio engine
python audio_interface.py --engine sounddevice --demo
```

### Web Server API

```bash
# Start web server
python web_server.py --host 0.0.0.0 --port 8080

# Start in debug mode
python web_server.py --debug
```

## API Endpoints

- `GET /api/presets` - List all presets
- `GET /api/presets/{name}` - Get preset details
- `POST /api/presets/{name}/play` - Play a preset
- `POST /api/presets/{name}/stop` - Stop a preset
- `POST /api/stop-all` - Stop all sounds
- `GET /api/status` - Get system status
- `GET /api/categories` - Get preset categories

## Project Structure

```
├── Makefile                 # Build system
├── audio_interface.py       # Core audio interface
├── web_server.py           # Flask web server
├── demo.py                 # Demo script
├── requirements.txt        # Python dependencies
├── web/
│   ├── package.json        # Web dependencies
│   ├── templates/          # HTML templates
│   └── static/            # CSS/JS assets
├── electronic_track.json   # Electronic sound presets
├── guitar.json            # Guitar sound presets
├── group.json             # Group sound presets
└── src/                   # C++ source code
```

## JSON Data Format

The system supports multiple JSON formats:

### Electronic Track Format
```json
{
  "instruments": {
    "Bass_Classic_MoogPunch": {
      "adsr": {
        "attack": "10ms",
        "decay": "200ms",
        "sustain": 0.8,
        "release": "300ms"
      },
      "effects": [
        {"type": "reverb", "amount": 0.3}
      ]
    }
  }
}
```

### Group Format
```json
{
  "groups": {
    "Pad_Warm_Calm": {
      "synthesis_type": "subtractive",
      "oscillator": {
        "types": ["sine", "triangle"],
        "mix_ratios": [0.7, 0.3]
      }
    }
  }
}
```

## Audio Engines

### SoundDevice (Recommended)
- High-quality audio playback
- Low latency
- Cross-platform support
- Requires: `pip install sounddevice soundfile`

### Pygame
- Simple audio playback
- Good for basic use cases
- Requires: `pip install pygame`

### Mock
- No actual audio output
- Useful for testing and development
- No additional dependencies

## Makefile Targets

### Build Targets
- `make all` - Build everything
- `make build-cpp` - Build C++ components
- `make build-python` - Install Python dependencies
- `make build-web` - Build web interface

### Development
- `make dev` - Start Python development server
- `make serve` - Start production web server
- `make serve-dev` - Start development web server

### Testing
- `make test` - Run all tests
- `make test-python` - Run Python tests
- `make test-cpp` - Run C++ tests

### Audio System
- `make audio-demo` - Run audio demonstration
- `make load-presets` - Load and display presets

### Installation
- `make install-deps` - Install all dependencies
- `make install` - Install application
- `make install-web` - Install web interface

### Cleanup
- `make clean` - Clean all build artifacts
- `make clean-cpp` - Clean C++ build
- `make clean-python` - Clean Python artifacts
- `make clean-web` - Clean web build

## Troubleshooting

### Audio Issues
- Ensure audio drivers are installed
- Check that audio devices are available
- Try different audio engines: `--engine sounddevice` or `--engine pygame`

### Web Interface Issues
- Check that port 8080 is available
- Try a different port: `--port 8081`
- Check browser console for JavaScript errors

### Build Issues
- Ensure all dependencies are installed: `make install-deps`
- Check that CMake version is 3.20 or higher
- Verify Python version is 3.8 or higher

### JSON Loading Issues
- Verify JSON files exist and are valid
- Check file permissions
- Look for error messages in the console

## Development

### Adding New Sound Presets
1. Create or modify JSON files with preset data
2. Follow the existing format structure
3. Restart the web server to load new presets

### Customizing the Web Interface
1. Modify `templates/index.html` for HTML changes
2. Update `static/css/style.css` for styling
3. Edit `static/js/app.js` for JavaScript functionality

### Extending Audio Generation
1. Modify `audio_interface.py` to add new synthesis methods
2. Update the `generate_audio_from_preset` method
3. Add new effect processing in `_apply_simple_effects`

## License

This project is part of the AI Audio Generator system. See the main project documentation for licensing information.

## Support

For issues and questions:
1. Check the troubleshooting section
2. Run `python demo.py` to test the system
3. Check the console output for error messages
4. Verify all dependencies are properly installed