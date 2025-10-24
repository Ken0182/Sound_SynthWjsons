"""
Core data models for JSON-to-Audio Preset Generator
Implements Step 1: Core data models and JSON schema mapping
"""

from dataclasses import dataclass, field
from typing import Dict, List, Optional, Union, Any, Tuple
from enum import Enum
import json
import math

class SynthesisType(Enum):
    SUBTRACTIVE = "subtractive"
    ADDITIVE = "additive"
    FM = "fm"
    WAVETABLE = "wavetable"
    GRANULAR = "granular"
    PHYSICAL_MODELING = "physical_modeling"
    MODULAR = "modular"
    HYBRID_AI = "hybrid_ai"
    ENSEMBLE_CHORUS = "ensemble_chorus"

class OscillatorType(Enum):
    SINE = "sine"
    SAWTOOTH = "sawtooth"
    TRIANGLE = "triangle"
    SQUARE = "square"
    PULSE = "pulse"
    NOISE = "noise"
    FM = "fm"
    WAVETABLE = "wavetable"
    SUPERSAW = "supersaw"
    GRANULAR = "granular"
    ADDITIVE = "additive"
    KARPLUS_STRONG = "karplus_strong"
    CHAOS_OSCILLATOR = "chaos_oscillator"
    NOISE_MODULATED = "noise_modulated"
    NEURAL_MORPH = "neural-morph"

class FilterType(Enum):
    LOW_PASS = "low-pass"
    HIGH_PASS = "high-pass"
    BAND_PASS = "band-pass"
    NOTCH = "notch"
    MOOG_LADDER = "moog_ladder"
    TB303 = "tb303"
    FORMANT = "formant"
    COMB = "comb"
    ENSEMBLE = "ensemble"
    MULTIMODE_ADAPTIVE = "multimode_adaptive"
    MULTIMODE_CHAOTIC = "multimode_chaotic"

class EnvelopeType(Enum):
    AD = "AD"
    ADSR = "ADSR"
    AHDSR = "AHDSR"
    DADSR = "DADSR"

class EnvelopeCurve(Enum):
    LINEAR = "linear"
    EXPONENTIAL = "exponential"
    LOGARITHMIC = "logarithmic"
    NONLINEAR = "nonlinear"

class Role(Enum):
    PAD = "pad"
    BASS = "bass"
    LEAD = "lead"
    FX = "fx"
    TEXTURE = "texture"
    ARP = "arp"
    DRONE = "drone"
    RHYTHM = "rhythm"
    BELL = "bell"
    CHORD = "chord"
    PLUCK = "pluck"

@dataclass
class OscillatorConfig:
    types: List[OscillatorType]
    mix_ratios: List[float]
    detune: float = 0.0
    modulation_index: Optional[float] = None
    carrier_ratio: Optional[float] = None
    harmonics: Optional[List[float]] = None
    morph_rate: Optional[str] = None
    table_index: Optional[str] = None
    grain_density: Optional[str] = None
    grain_size: Optional[str] = None
    pluck_position: Optional[str] = None
    blend_mode: Optional[str] = None

@dataclass
class EnvelopeConfig:
    type: EnvelopeType
    attack: Union[str, float, List[float]]  # ms or range
    decay: Union[str, float, List[float]] = 0.0
    sustain: Union[float, List[float]] = 1.0
    release: Union[str, float, List[float]] = 0.0
    hold: Union[str, float, List[float]] = 0.0
    delay: Union[str, float, List[float]] = 0.0
    curve: EnvelopeCurve = EnvelopeCurve.LINEAR

@dataclass
class FilterConfig:
    type: FilterType
    cutoff: Union[str, float, List[float]]  # Hz or range
    resonance: Union[float, List[float]] = 0.0
    envelope_amount: Union[float, List[float]] = 0.0
    slope: str = "12dB/oct"

@dataclass
class EffectConfig:
    type: str
    mix: Union[float, List[float]] = 1.0
    # Common effect parameters
    feedback: Optional[Union[float, List[float]]] = None
    time: Optional[Union[str, float, List[float]]] = None
    gain: Optional[Union[float, List[float]]] = None
    amount: Optional[Union[float, List[float]]] = None
    decay: Optional[Union[str, float, List[float]]] = None
    wet: Optional[Union[float, List[float]]] = None
    rate: Optional[Union[str, float, List[float]]] = None
    depth: Optional[Union[float, List[float]]] = None
    frequency: Optional[Union[str, float, List[float]]] = None
    density: Optional[Union[float, List[float]]] = None
    threshold: Optional[Union[float, List[float]]] = None
    ratio: Optional[Union[float, List[float]]] = None

@dataclass
class SoundCharacteristics:
    timbral: str
    material: str
    emotional: List[Dict[str, Union[str, float]]]
    dynamic: str

@dataclass
class TopologicalMetadata:
    damping: str
    spectral_complexity: str
    manifold_position: str

@dataclass
class AIControl:
    enabled: bool = True
    modulation_depth: float = 1.0
    jitter_amount: float = 0.1
    morphing_rate: float = 0.5

@dataclass
class JsonPreset:
    """Main preset data structure"""
    name: str
    synthesis_type: SynthesisType
    oscillator: OscillatorConfig
    envelope: EnvelopeConfig
    filter: FilterConfig
    fx: List[EffectConfig] = field(default_factory=list)
    sound_characteristics: SoundCharacteristics = None
    topological_metadata: TopologicalMetadata = None
    ai_control: AIControl = field(default_factory=AIControl)
    role: Optional[Role] = None
    metadata: Dict[str, Any] = field(default_factory=dict)

@dataclass
class NormalizedPreset:
    """Normalized preset with SI units and canonical ranges"""
    name: str
    synthesis_type: SynthesisType
    oscillator: OscillatorConfig
    envelope: EnvelopeConfig
    filter: FilterConfig
    fx: List[EffectConfig]
    sound_characteristics: SoundCharacteristics
    topological_metadata: TopologicalMetadata
    ai_control: AIControl
    role: Role
    metadata: Dict[str, Any]
    validation_issues: List[str] = field(default_factory=list)

@dataclass
class GraphNode:
    """DSP graph node"""
    id: str
    node_type: str
    parameters: Dict[str, float]
    inputs: List[str] = field(default_factory=list)
    outputs: List[str] = field(default_factory=list)

@dataclass
class Graph:
    """Deterministic Control Graph"""
    nodes: List[GraphNode] = field(default_factory=list)
    connections: List[Tuple[str, str, str, str]] = field(default_factory=list)  # (from_node, from_port, to_node, to_port)
    virtual_busses: Dict[str, List[str]] = field(default_factory=dict)  # bus_name -> connected_nodes
    validation_passed: bool = False
    validation_errors: List[str] = field(default_factory=list)

class JsonPresetParser:
    """Parser for converting JSON data to JsonPreset objects"""
    
    @staticmethod
    def parse_from_file(file_path: str) -> List[JsonPreset]:
        """Parse JSON file and return list of presets"""
        with open(file_path, 'r') as f:
            data = json.load(f)
        
        presets = []
        
        # Handle different JSON structures
        if 'instruments' in data:
            # Electronic track format
            for name, config in data['instruments'].items():
                preset = JsonPresetParser._parse_instrument_config(name, config)
                presets.append(preset)
        elif 'groups' in data:
            # Group format
            for name, config in data['groups'].items():
                preset = JsonPresetParser._parse_group_config(name, config)
                presets.append(preset)
        elif 'guitar_types' in data:
            # Guitar format
            for guitar_type, type_data in data['guitar_types'].items():
                for group_name, group_config in type_data.get('groups', {}).items():
                    preset = JsonPresetParser._parse_guitar_config(group_name, group_config, guitar_type)
                    presets.append(preset)
        
        return presets
    
    @staticmethod
    def _parse_instrument_config(name: str, config: Dict) -> JsonPreset:
        """Parse instrument configuration"""
        return JsonPreset(
            name=name,
            synthesis_type=SynthesisType(config.get('synthesisType', 'subtractive')),
            oscillator=JsonPresetParser._parse_oscillator(config.get('oscillator', {})),
            envelope=JsonPresetParser._parse_envelope(config.get('adsr', {})),
            filter=JsonPresetParser._parse_filter(config.get('filter', {})),
            fx=[JsonPresetParser._parse_effect(eff) for eff in config.get('effects', [])],
            sound_characteristics=JsonPresetParser._parse_sound_characteristics(config.get('soundCharacteristics', {})),
            topological_metadata=JsonPresetParser._parse_topological_metadata(config.get('topologicalMetadata', {})),
            metadata=config.get('metadata', {})
        )
    
    @staticmethod
    def _parse_group_config(name: str, config: Dict) -> JsonPreset:
        """Parse group configuration"""
        return JsonPreset(
            name=name,
            synthesis_type=SynthesisType(config.get('synthesis_type', 'subtractive')),
            oscillator=JsonPresetParser._parse_oscillator(config.get('oscillator', {})),
            envelope=JsonPresetParser._parse_envelope(config.get('envelope', {})),
            filter=JsonPresetParser._parse_filter(config.get('filter', {})),
            fx=[JsonPresetParser._parse_effect(eff) for eff in config.get('fx', [])],
            sound_characteristics=JsonPresetParser._parse_sound_characteristics(config.get('sound_characteristics', {})),
            topological_metadata=JsonPresetParser._parse_topological_metadata(config.get('topological_metadata', {})),
            metadata={}
        )
    
    @staticmethod
    def _parse_guitar_config(name: str, config: Dict, guitar_type: str) -> JsonPreset:
        """Parse guitar configuration"""
        return JsonPreset(
            name=name,
            synthesis_type=SynthesisType('physical_modeling'),
            oscillator=JsonPresetParser._parse_guitar_oscillator(config.get('strings', {}), config.get('harmonics', {})),
            envelope=JsonPresetParser._parse_envelope(config.get('envelope', {})),
            filter=JsonPresetParser._parse_filter(config.get('filter', {})),
            fx=[JsonPresetParser._parse_effect(eff) for eff in config.get('fx', [])],
            sound_characteristics=JsonPresetParser._parse_sound_characteristics(config.get('sound_characteristics', {})),
            topological_metadata=JsonPresetParser._parse_topological_metadata(config.get('topological_metadata', {})),
            metadata={'guitar_type': guitar_type}
        )
    
    @staticmethod
    def _parse_oscillator(config: Dict) -> OscillatorConfig:
        """Parse oscillator configuration"""
        types = [OscillatorType(t) for t in config.get('types', ['sine'])]
        mix_ratios = config.get('mix_ratios', [1.0])
        
        return OscillatorConfig(
            types=types,
            mix_ratios=mix_ratios,
            detune=config.get('detune', 0.0),
            modulation_index=config.get('modulation_index'),
            carrier_ratio=config.get('carrier_ratio'),
            harmonics=config.get('harmonics'),
            morph_rate=config.get('morph_rate'),
            table_index=config.get('table_index'),
            grain_density=config.get('grain_density'),
            grain_size=config.get('grain_size'),
            pluck_position=config.get('pluck_position'),
            blend_mode=config.get('blend_mode')
        )
    
    @staticmethod
    def _parse_guitar_oscillator(strings_config: Dict, harmonics_config: Dict) -> OscillatorConfig:
        """Parse guitar-specific oscillator configuration"""
        return OscillatorConfig(
            types=[OscillatorType.KARPLUS_STRONG],
            mix_ratios=[1.0],
            detune=0.0,
            harmonics=harmonics_config.get('vibe_set', [1.0])
        )
    
    @staticmethod
    def _parse_envelope(config: Dict) -> EnvelopeConfig:
        """Parse envelope configuration"""
        return EnvelopeConfig(
            type=EnvelopeType(config.get('type', 'ADSR')),
            attack=config.get('attack', 0.0),
            decay=config.get('decay', 0.0),
            sustain=config.get('sustain', 1.0),
            release=config.get('release', 0.0),
            hold=config.get('hold', 0.0),
            delay=config.get('delay', 0.0),
            curve=EnvelopeCurve(config.get('curve', 'linear'))
        )
    
    @staticmethod
    def _parse_filter(config: Dict) -> FilterConfig:
        """Parse filter configuration"""
        return FilterConfig(
            type=FilterType(config.get('type', 'low-pass')),
            cutoff=config.get('cutoff', 1000.0),
            resonance=config.get('resonance', 0.0),
            envelope_amount=config.get('envelope_amount', 0.0),
            slope=config.get('slope', '12dB/oct')
        )
    
    @staticmethod
    def _parse_effect(config: Dict) -> EffectConfig:
        """Parse effect configuration"""
        return EffectConfig(
            type=config.get('type', 'reverb'),
            mix=config.get('mix', 1.0),
            feedback=config.get('feedback'),
            time=config.get('time'),
            gain=config.get('gain'),
            amount=config.get('amount'),
            decay=config.get('decay'),
            wet=config.get('wet'),
            rate=config.get('rate'),
            depth=config.get('depth'),
            frequency=config.get('frequency'),
            density=config.get('density'),
            threshold=config.get('threshold'),
            ratio=config.get('ratio')
        )
    
    @staticmethod
    def _parse_sound_characteristics(config: Dict) -> SoundCharacteristics:
        """Parse sound characteristics"""
        return SoundCharacteristics(
            timbral=config.get('timbral', 'neutral'),
            material=config.get('material', 'synthetic'),
            emotional=config.get('emotional', []),
            dynamic=config.get('dynamic', 'sustained')
        )
    
    @staticmethod
    def _parse_topological_metadata(config: Dict) -> TopologicalMetadata:
        """Parse topological metadata"""
        return TopologicalMetadata(
            damping=config.get('damping', 'medium'),
            spectral_complexity=config.get('spectral_complexity', 'medium'),
            manifold_position=config.get('manifold_position', 'center')
        )

if __name__ == "__main__":
    # Test the parser with the provided JSON files
    parser = JsonPresetParser()
    
    # Parse all JSON files
    files = ['electronic_track.json', 'group.json', 'guitar.json']
    all_presets = []
    
    for file in files:
        try:
            presets = parser.parse_from_file(file)
            all_presets.extend(presets)
            print(f"Parsed {len(presets)} presets from {file}")
        except Exception as e:
            print(f"Error parsing {file}: {e}")
    
    print(f"Total presets parsed: {len(all_presets)}")
    
    # Print sample preset info
    if all_presets:
        sample = all_presets[0]
        print(f"\nSample preset: {sample.name}")
        print(f"  Synthesis type: {sample.synthesis_type}")
        print(f"  Oscillator types: {[t.value for t in sample.oscillator.types]}")
        print(f"  Filter type: {sample.filter.type}")
        print(f"  Effects: {len(sample.fx)}")