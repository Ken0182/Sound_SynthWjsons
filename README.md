# JSON-to-Audio Preset Generator

A comprehensive system for converting JSON-based synthesizer presets into playable DSP graphs with semantic search and AI-driven parameter adjustment.

## Overview

This system implements a 20-step plan for creating a robust JSON data analyzer and sound outputter that can:

- Parse various JSON preset formats
- Normalize parameters to SI units and canonical ranges
- Build deterministic control graphs for audio synthesis
- Perform semantic search on preset databases
- Apply role-based style policies
- Generate parameter decisions using AI-driven heads
- Validate and ensure signal safety

## Features

### Core Functionality
- **Multi-format JSON parsing** - Supports electronic tracks, group presets, and guitar configurations
- **Unit normalization** - Converts all parameters to SI units with safety constraints
- **Deterministic graph building** - Creates playable DSP graphs from JSON presets
- **Semantic search** - Find presets using natural language queries
- **Role-based policies** - Apply musical role constraints (Pad, Bass, Lead, FX, etc.)
- **AI decision heads** - Generate parameter values and modulation routing
- **Signal safety** - Ensure all outputs are within safe ranges

### Supported Roles
- **Pad** - Warm, sustained atmospheric sounds
- **Bass** - Punchy, driving low-frequency sounds
- **Lead** - Bright, energetic melodic sounds
- **FX** - Chaotic, experimental effects
- **Texture** - Evolving, ambient soundscapes
- **Arp** - Rhythmic, plucky arpeggiated sounds
- **Drone** - Long, sustained ambient tones
- **Rhythm** - Percussive, rhythmic elements
- **Bell** - Glassy, crystalline sounds
- **Chord** - Lush, harmonic chordal sounds
- **Pluck** - Physical modeling string sounds

## Installation

1. Clone the repository
2. Install dependencies:
```bash
pip install -r requirements.txt
```

## Usage

### Basic Usage

```bash
# Generate a preset from a text query
python main.py --query "warm analog pad" --role pad --tempo 120 --key C

# Search for presets without generating
python main.py --query "bright lead" --search-only

# Generate with explanation
python main.py --query "punchy bass" --role bass --explain
```

### Command Line Options

- `--query` - Text description of desired sound (required)
- `--role` - Musical role (pad, bass, lead, fx, etc.)
- `--tempo` - Tempo in BPM
- `--key` - Musical key
- `--files` - JSON files to load (default: group.json, electronic_track.json, guitar.json)
- `--search-only` - Only perform search, don't generate preset
- `--explain` - Generate human-readable explanation

### Programmatic Usage

```python
from main import AudioPresetGenerator

# Create generator
generator = AudioPresetGenerator()

# Load presets
generator.load_presets(["group.json", "electronic_track.json"])

# Normalize and build search index
generator.normalize_presets()
generator.build_search_index()

# Search for presets
results = generator.search_presets("warm analog pad", role="pad", top_k=5)

# Generate complete preset
preset = generator.generate_audio_preset(
    query="bright energetic lead",
    role="lead",
    tempo=140,
    key="G"
)

# Get explanation
explanation = generator.explain_preset(preset)
print(explanation)
```

## Architecture

### Core Components

1. **Core Models** (`core_models.py`)
   - Data structures for presets, envelopes, filters, effects
   - JSON parsing for multiple formats
   - Type definitions and enums

2. **Normalizer** (`normalizer.py`)
   - Unit conversion (ms→s, Hz, dB, etc.)
   - Parameter range validation
   - Safety constraint enforcement
   - Role-based default application

3. **Graph Builder** (`graph_builder.py`)
   - DSP graph construction
   - Signal chain ordering
   - Modulation routing
   - Feedback loop detection

4. **Semantic Embeddings** (`semantic_embeddings.py`)
   - Text tokenization and canonicalization
   - Vector embedding generation
   - Similarity search
   - Query processing

5. **Role Policies** (`roles_policies.py`)
   - Role-specific parameter ranges
   - Envelope, filter, and spatial policies
   - Tempo and key awareness
   - Policy precedence handling

6. **Decision Heads** (`decision_heads.py`)
   - Parameter value inference
   - Modulation routing decisions
   - Jitter and randomization
   - Context-aware adjustments

### Data Flow

```
JSON Files → Parser → Normalizer → Search Index
                                    ↓
Query → Semantic Search → Best Match → Role Policy → Decision Heads → DSP Graph
```

## JSON Format Support

### Electronic Track Format
```json
{
  "instruments": {
    "PresetName": {
      "synthesisType": "subtractive",
      "oscillator": {...},
      "adsr": {...},
      "filter": {...},
      "effects": [...],
      "soundCharacteristics": {...}
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
      "oscillator": {...},
      "envelope": {...},
      "filter": {...},
      "fx": [...],
      "sound_characteristics": {...}
    }
  }
}
```

### Guitar Format
```json
{
  "guitar_types": {
    "acoustic": {
      "groups": {
        "Acoustic_Warm_Fingerstyle": {
          "strings": {...},
          "harmonics": {...},
          "envelope": {...},
          "filter": {...}
        }
      }
    }
  }
}
```

## Safety Features

- **Parameter clamping** - All parameters stay within safe ranges
- **Feedback limiting** - Delay feedback capped at 0.85
- **Frequency limits** - Filter cutoff between 20-20000 Hz
- **Resonance limits** - Filter resonance capped at 0.9
- **Gain limiting** - Distortion gain capped at +12 dB
- **Graph validation** - Prevents feedback loops and invalid connections

## Testing

Run the test suite to verify all components:

```bash
python test_system.py
```

This will test:
- JSON parsing and loading
- Parameter normalization
- Search index building
- Semantic search functionality
- Preset generation
- Individual component functionality

## Examples

### Generate a Warm Pad
```bash
python main.py --query "warm analog pad with reverb" --role pad --tempo 100 --key Am
```

### Find Bass Presets
```bash
python main.py --query "punchy analog bass" --role bass --search-only
```

### Create a Lead Sound
```bash
python main.py --query "bright supersaw lead" --role lead --tempo 140 --key G --explain
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Roadmap

Future enhancements planned:
- Real-time audio generation
- VST plugin integration
- Advanced AI model training
- Web interface
- Preset sharing and collaboration
- Advanced modulation routing
- Multi-timbral support
- MIDI input handling