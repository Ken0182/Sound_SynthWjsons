#!/usr/bin/env python3
"""
Preset Serializer for AI Audio Generator
Handles serialization and deserialization of preset data according to the shared schema
"""

import json
import yaml
from typing import Dict, Any, List, Optional, Union
from dataclasses import dataclass, asdict
from datetime import datetime
import jsonschema
from pathlib import Path

@dataclass
class OscillatorParams:
    frequency: float = 440.0
    waveform: str = "sine"
    detune: float = 0.0
    phase: float = 0.0

@dataclass
class EnvelopeParams:
    attack: float = 0.1
    decay: float = 0.1
    sustain: float = 0.7
    release: float = 0.5

@dataclass
class FilterParams:
    type: str = "lowpass"
    cutoff: float = 1000.0
    resonance: float = 1.0
    slope: int = 12

@dataclass
class EffectParams:
    type: str
    amount: float = 0.5
    parameters: Dict[str, Any] = None

@dataclass
class ModulationParams:
    lfo_rate: float = 1.0
    lfo_depth: float = 0.1
    lfo_target: str = "frequency"

@dataclass
class PerformanceParams:
    polyphony: int = 8
    voice_stealing: bool = True
    portamento: float = 0.0

@dataclass
class QualityParams:
    sample_rate: int = 44100
    bit_depth: int = 24
    oversampling: int = 1

@dataclass
class PresetMetadata:
    author: str = ""
    version: str = "1.0.0"
    tags: List[str] = None
    created: str = ""
    modified: str = ""

@dataclass
class PresetParameters:
    oscillator: OscillatorParams = None
    envelope: EnvelopeParams = None
    filter: FilterParams = None
    effects: List[EffectParams] = None
    modulation: ModulationParams = None
    performance: PerformanceParams = None
    quality: QualityParams = None
    metadata: PresetMetadata = None

@dataclass
class Preset:
    name: str
    category: str
    description: str = ""
    role: str = "UNKNOWN"
    parameters: PresetParameters = None

class PresetSerializer:
    """Handles serialization and deserialization of preset data"""
    
    def __init__(self, schema_path: str = "config/preset_schema.yaml"):
        self.schema_path = schema_path
        self.schema = self._load_schema()
    
    def _load_schema(self) -> Dict[str, Any]:
        """Load the JSON schema for validation"""
        try:
            with open(self.schema_path, 'r') as f:
                return yaml.safe_load(f)
        except FileNotFoundError:
            print(f"Warning: Schema file not found at {self.schema_path}")
            return {}
    
    def validate_preset(self, preset_data: Dict[str, Any]) -> List[str]:
        """Validate preset data against the schema"""
        errors = []
        
        try:
            if self.schema:
                jsonschema.validate(preset_data, self.schema.get('preset', {}))
        except jsonschema.ValidationError as e:
            errors.append(f"Schema validation error: {e.message}")
        except Exception as e:
            errors.append(f"Validation error: {e}")
        
        return errors
    
    def serialize_preset(self, preset: Preset) -> Dict[str, Any]:
        """Serialize a Preset object to a dictionary"""
        # Convert dataclass to dict
        preset_dict = asdict(preset)
        
        # Ensure parameters is properly structured
        if preset_dict['parameters'] is None:
            preset_dict['parameters'] = {}
        else:
            # Convert nested dataclasses to dicts
            params = preset_dict['parameters']
            if isinstance(params, dict):
                for key, value in params.items():
                    if hasattr(value, '__dict__'):
                        params[key] = asdict(value)
            else:
                preset_dict['parameters'] = asdict(params)
        
        # Add timestamps if not present
        if 'metadata' in preset_dict['parameters']:
            metadata = preset_dict['parameters']['metadata']
            if not metadata.get('created'):
                metadata['created'] = datetime.now().isoformat()
            if not metadata.get('modified'):
                metadata['modified'] = datetime.now().isoformat()
        
        return preset_dict
    
    def deserialize_preset(self, preset_data: Dict[str, Any]) -> Preset:
        """Deserialize a dictionary to a Preset object"""
        # Validate the data
        errors = self.validate_preset(preset_data)
        if errors:
            print(f"Validation errors: {errors}")
        
        # Extract basic fields
        name = preset_data.get('name', '')
        category = preset_data.get('category', '')
        description = preset_data.get('description', '')
        role = preset_data.get('role', 'UNKNOWN')
        
        # Extract parameters
        params_data = preset_data.get('parameters', {})
        parameters = self._deserialize_parameters(params_data)
        
        return Preset(
            name=name,
            category=category,
            description=description,
            role=role,
            parameters=parameters
        )
    
    def _deserialize_parameters(self, params_data: Dict[str, Any]) -> PresetParameters:
        """Deserialize parameters from dictionary"""
        # Oscillator
        oscillator_data = params_data.get('oscillator', {})
        oscillator = OscillatorParams(
            frequency=oscillator_data.get('frequency', 440.0),
            waveform=oscillator_data.get('waveform', 'sine'),
            detune=oscillator_data.get('detune', 0.0),
            phase=oscillator_data.get('phase', 0.0)
        )
        
        # Envelope
        envelope_data = params_data.get('envelope', {})
        envelope = EnvelopeParams(
            attack=envelope_data.get('attack', 0.1),
            decay=envelope_data.get('decay', 0.1),
            sustain=envelope_data.get('sustain', 0.7),
            release=envelope_data.get('release', 0.5)
        )
        
        # Filter
        filter_data = params_data.get('filter', {})
        filter_params = FilterParams(
            type=filter_data.get('type', 'lowpass'),
            cutoff=filter_data.get('cutoff', 1000.0),
            resonance=filter_data.get('resonance', 1.0),
            slope=filter_data.get('slope', 12)
        )
        
        # Effects
        effects_data = params_data.get('effects', [])
        effects = []
        for effect_data in effects_data:
            effect = EffectParams(
                type=effect_data.get('type', ''),
                amount=effect_data.get('amount', 0.5),
                parameters=effect_data.get('parameters', {})
            )
            effects.append(effect)
        
        # Modulation
        modulation_data = params_data.get('modulation', {})
        modulation = ModulationParams(
            lfo_rate=modulation_data.get('lfo_rate', 1.0),
            lfo_depth=modulation_data.get('lfo_depth', 0.1),
            lfo_target=modulation_data.get('lfo_target', 'frequency')
        )
        
        # Performance
        performance_data = params_data.get('performance', {})
        performance = PerformanceParams(
            polyphony=performance_data.get('polyphony', 8),
            voice_stealing=performance_data.get('voice_stealing', True),
            portamento=performance_data.get('portamento', 0.0)
        )
        
        # Quality
        quality_data = params_data.get('quality', {})
        quality = QualityParams(
            sample_rate=quality_data.get('sample_rate', 44100),
            bit_depth=quality_data.get('bit_depth', 24),
            oversampling=quality_data.get('oversampling', 1)
        )
        
        # Metadata
        metadata_data = params_data.get('metadata', {})
        metadata = PresetMetadata(
            author=metadata_data.get('author', ''),
            version=metadata_data.get('version', '1.0.0'),
            tags=metadata_data.get('tags', []),
            created=metadata_data.get('created', ''),
            modified=metadata_data.get('modified', '')
        )
        
        return PresetParameters(
            oscillator=oscillator,
            envelope=envelope,
            filter=filter_params,
            effects=effects,
            modulation=modulation,
            performance=performance,
            quality=quality,
            metadata=metadata
        )
    
    def save_preset(self, preset: Preset, file_path: str) -> bool:
        """Save a preset to a JSON file"""
        try:
            preset_dict = self.serialize_preset(preset)
            
            # Ensure directory exists
            Path(file_path).parent.mkdir(parents=True, exist_ok=True)
            
            with open(file_path, 'w') as f:
                json.dump(preset_dict, f, indent=2, default=str)
            
            return True
        except Exception as e:
            print(f"Error saving preset: {e}")
            return False
    
    def load_preset(self, file_path: str) -> Optional[Preset]:
        """Load a preset from a JSON file"""
        try:
            with open(file_path, 'r') as f:
                preset_data = json.load(f)
            
            return self.deserialize_preset(preset_data)
        except Exception as e:
            print(f"Error loading preset: {e}")
            return None
    
    def convert_legacy_preset(self, legacy_data: Dict[str, Any]) -> Preset:
        """Convert legacy preset format to new schema-compliant format"""
        # Extract basic information
        name = legacy_data.get('name', 'Unknown')
        category = legacy_data.get('category', 'electronic')
        description = legacy_data.get('description', '')
        
        # Convert parameters
        parameters = PresetParameters()
        
        # Map common legacy parameters
        if 'frequency' in legacy_data:
            parameters.oscillator.frequency = float(legacy_data['frequency'])
        
        if 'waveform' in legacy_data:
            parameters.oscillator.waveform = str(legacy_data['waveform'])
        
        if 'adsr' in legacy_data:
            adsr = legacy_data['adsr']
            parameters.envelope.attack = float(adsr.get('attack', 0.1))
            parameters.envelope.decay = float(adsr.get('decay', 0.1))
            parameters.envelope.sustain = float(adsr.get('sustain', 0.7))
            parameters.envelope.release = float(adsr.get('release', 0.5))
        
        if 'filter' in legacy_data:
            filt = legacy_data['filter']
            parameters.filter.cutoff = float(filt.get('cutoff', 1000.0))
            parameters.filter.resonance = float(filt.get('resonance', 1.0))
        
        # Add metadata
        parameters.metadata.created = datetime.now().isoformat()
        parameters.metadata.modified = datetime.now().isoformat()
        
        return Preset(
            name=name,
            category=category,
            description=description,
            role="UNKNOWN",
            parameters=parameters
        )

def main():
    """Test the preset serializer"""
    serializer = PresetSerializer()
    
    # Create a test preset
    test_preset = Preset(
        name="Test Pad",
        category="electronic",
        description="A warm pad sound",
        role="PAD",
        parameters=PresetParameters(
            oscillator=OscillatorParams(frequency=220.0, waveform="sawtooth"),
            envelope=EnvelopeParams(attack=0.5, sustain=0.8),
            filter=FilterParams(cutoff=800.0, resonance=0.5)
        )
    )
    
    # Serialize and save
    print("Serializing preset...")
    success = serializer.save_preset(test_preset, "test_preset.json")
    if success:
        print("✓ Preset saved successfully")
    
    # Load and validate
    print("Loading preset...")
    loaded_preset = serializer.load_preset("test_preset.json")
    if loaded_preset:
        print("✓ Preset loaded successfully")
        print(f"Name: {loaded_preset.name}")
        print(f"Category: {loaded_preset.category}")
        print(f"Frequency: {loaded_preset.parameters.oscillator.frequency}")
    
    # Test legacy conversion
    print("Testing legacy conversion...")
    legacy_data = {
        "name": "Legacy Sound",
        "category": "guitar",
        "frequency": 440.0,
        "waveform": "sine",
        "adsr": {
            "attack": 0.1,
            "decay": 0.2,
            "sustain": 0.7,
            "release": 0.5
        }
    }
    
    converted_preset = serializer.convert_legacy_preset(legacy_data)
    print(f"✓ Converted preset: {converted_preset.name}")

if __name__ == "__main__":
    main()