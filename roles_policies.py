"""
Roles & Style Policies System
Implements Step 5: Roles & style policies (Pad/FX/Bass/Lead)
"""

from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass, field
from enum import Enum
import yaml
import json
from core_models import Role, EnvelopeType, EnvelopeCurve, FilterType, SynthesisType

@dataclass
class EnvelopePolicy:
    """Envelope parameter ranges for a role"""
    attack_min: float  # seconds
    attack_max: float
    decay_min: float
    decay_max: float
    sustain_min: float
    sustain_max: float
    release_min: float
    release_max: float
    curve: EnvelopeCurve
    type: EnvelopeType
    tempo_scaling: bool = True

@dataclass
class FilterPolicy:
    """Filter parameter ranges for a role"""
    cutoff_min: float  # Hz
    cutoff_max: float
    resonance_min: float
    resonance_max: float
    envelope_amount_min: float
    envelope_amount_max: float
    preferred_type: FilterType

@dataclass
class SpatialPolicy:
    """Spatial processing parameters for a role"""
    stereo_width: float  # 0.0 = mono, 1.0 = full stereo
    motion_amount: float  # 0.0 = static, 1.0 = high motion
    mono_compatible: bool
    reverb_amount: float  # 0.0 = dry, 1.0 = wet
    delay_amount: float  # 0.0 = no delay, 1.0 = high delay

@dataclass
class ModulationPolicy:
    """Modulation parameters for a role"""
    lfo_rate_min: float  # Hz
    lfo_rate_max: float
    vibrato_freq_min: float  # Hz
    vibrato_freq_max: float
    vibrato_depth_min: float  # cents
    vibrato_depth_max: float
    tremolo_amount: float  # 0.0 = none, 1.0 = high

@dataclass
class RolePolicy:
    """Complete policy for a musical role"""
    role: Role
    envelope: EnvelopePolicy
    filter: FilterPolicy
    spatial: SpatialPolicy
    modulation: ModulationPolicy
    synthesis_preferences: List[SynthesisType]
    tempo_scaling: bool  # Whether to scale envelope times with tempo
    key_awareness: bool  # Whether to adjust parameters based on key
    priority: int  # Higher number = higher priority when conflicts occur

class RolePolicyManager:
    """Manages role-based policies and their application"""
    
    def __init__(self):
        self.policies: Dict[Role, RolePolicy] = {}
        self._load_default_policies()
    
    def get_policy(self, role: Role) -> RolePolicy:
        """Get policy for a specific role"""
        return self.policies.get(role, self.policies[Role.PAD])  # Default to pad
    
    def apply_policy(self, preset, role: Role, tempo: float = 120.0, key: str = "C") -> Dict[str, Any]:
        """Apply role policy to a preset"""
        policy = self.get_policy(role)
        adjustments = {}
        
        # Apply envelope policy
        if hasattr(preset, 'envelope'):
            adjustments['envelope'] = self._apply_envelope_policy(preset.envelope, policy.envelope, tempo)
        
        # Apply filter policy
        if hasattr(preset, 'filter'):
            adjustments['filter'] = self._apply_filter_policy(preset.filter, policy.filter)
        
        # Apply spatial policy
        adjustments['spatial'] = self._apply_spatial_policy(policy.spatial)
        
        # Apply modulation policy
        adjustments['modulation'] = self._apply_modulation_policy(policy.modulation)
        
        # Apply synthesis type preferences
        if hasattr(preset, 'synthesis_type'):
            adjustments['synthesis_type'] = self._apply_synthesis_preferences(
                preset.synthesis_type, policy.synthesis_preferences
            )
        
        return adjustments
    
    def _apply_envelope_policy(self, envelope, policy: EnvelopePolicy, tempo: float) -> Dict[str, Any]:
        """Apply envelope policy with tempo scaling"""
        adjustments = {}
        
        # Scale times with tempo if enabled
        tempo_factor = 120.0 / tempo if policy.tempo_scaling else 1.0
        
        # Adjust attack time
        if hasattr(envelope, 'attack'):
            current_attack = self._parse_time_to_seconds(envelope.attack)
            target_attack = self._clamp_to_range(
                current_attack, 
                policy.attack_min * tempo_factor, 
                policy.attack_max * tempo_factor
            )
            adjustments['attack'] = target_attack
        
        # Adjust decay time
        if hasattr(envelope, 'decay'):
            current_decay = self._parse_time_to_seconds(envelope.decay)
            target_decay = self._clamp_to_range(
                current_decay,
                policy.decay_min * tempo_factor,
                policy.decay_max * tempo_factor
            )
            adjustments['decay'] = target_decay
        
        # Adjust sustain level
        if hasattr(envelope, 'sustain'):
            current_sustain = envelope.sustain if isinstance(envelope.sustain, float) else 0.7
            target_sustain = self._clamp_to_range(
                current_sustain,
                policy.sustain_min,
                policy.sustain_max
            )
            adjustments['sustain'] = target_sustain
        
        # Adjust release time
        if hasattr(envelope, 'release'):
            current_release = self._parse_time_to_seconds(envelope.release)
            target_release = self._clamp_to_range(
                current_release,
                policy.release_min * tempo_factor,
                policy.release_max * tempo_factor
            )
            adjustments['release'] = target_release
        
        # Set curve and type
        adjustments['curve'] = policy.curve
        adjustments['type'] = policy.type
        
        return adjustments
    
    def _apply_filter_policy(self, filter_config, policy: FilterPolicy) -> Dict[str, Any]:
        """Apply filter policy"""
        adjustments = {}
        
        # Adjust cutoff frequency
        if hasattr(filter_config, 'cutoff'):
            current_cutoff = self._parse_frequency_to_hz(filter_config.cutoff)
            target_cutoff = self._clamp_to_range(
                current_cutoff,
                policy.cutoff_min,
                policy.cutoff_max
            )
            adjustments['cutoff'] = target_cutoff
        
        # Adjust resonance
        if hasattr(filter_config, 'resonance'):
            current_resonance = filter_config.resonance if isinstance(filter_config.resonance, float) else 0.5
            target_resonance = self._clamp_to_range(
                current_resonance,
                policy.resonance_min,
                policy.resonance_max
            )
            adjustments['resonance'] = target_resonance
        
        # Adjust envelope amount
        if hasattr(filter_config, 'envelope_amount'):
            current_env_amount = filter_config.envelope_amount if isinstance(filter_config.envelope_amount, float) else 0.0
            target_env_amount = self._clamp_to_range(
                current_env_amount,
                policy.envelope_amount_min,
                policy.envelope_amount_max
            )
            adjustments['envelope_amount'] = target_env_amount
        
        # Set preferred filter type
        adjustments['type'] = policy.preferred_type
        
        return adjustments
    
    def _apply_spatial_policy(self, policy: SpatialPolicy) -> Dict[str, Any]:
        """Apply spatial processing policy"""
        return {
            'stereo_width': policy.stereo_width,
            'motion_amount': policy.motion_amount,
            'mono_compatible': policy.mono_compatible,
            'reverb_amount': policy.reverb_amount,
            'delay_amount': policy.delay_amount
        }
    
    def _apply_modulation_policy(self, policy: ModulationPolicy) -> Dict[str, Any]:
        """Apply modulation policy"""
        return {
            'lfo_rate_min': policy.lfo_rate_min,
            'lfo_rate_max': policy.lfo_rate_max,
            'vibrato_freq_min': policy.vibrato_freq_min,
            'vibrato_freq_max': policy.vibrato_freq_max,
            'vibrato_depth_min': policy.vibrato_depth_min,
            'vibrato_depth_max': policy.vibrato_depth_max,
            'tremolo_amount': policy.tremolo_amount
        }
    
    def _apply_synthesis_preferences(self, current_type: SynthesisType, preferences: List[SynthesisType]) -> SynthesisType:
        """Apply synthesis type preferences"""
        if current_type in preferences:
            return current_type
        else:
            return preferences[0]  # Use first preference as fallback
    
    def _clamp_to_range(self, value: float, min_val: float, max_val: float) -> float:
        """Clamp value to range"""
        return max(min_val, min(value, max_val))
    
    def _parse_time_to_seconds(self, time_value) -> float:
        """Parse time value to seconds"""
        if isinstance(time_value, (int, float)):
            return float(time_value) / 1000.0  # Assume milliseconds
        elif isinstance(time_value, str):
            if time_value.endswith('ms'):
                return float(time_value[:-2]) / 1000.0
            elif time_value.endswith('s'):
                return float(time_value[:-1])
            else:
                return float(time_value) / 1000.0
        elif isinstance(time_value, list):
            return (time_value[0] + time_value[1]) / 2.0 / 1000.0
        return 0.0
    
    def _parse_frequency_to_hz(self, freq_value) -> float:
        """Parse frequency value to Hz"""
        if isinstance(freq_value, (int, float)):
            return float(freq_value)
        elif isinstance(freq_value, str):
            if freq_value.endswith('Hz'):
                return float(freq_value[:-2])
            elif freq_value.endswith('kHz'):
                return float(freq_value[:-3]) * 1000.0
            else:
                return float(freq_value)
        elif isinstance(freq_value, list):
            return (freq_value[0] + freq_value[1]) / 2.0
        return 1000.0
    
    def _load_default_policies(self):
        """Load default role policies"""
        # Pad policy
        self.policies[Role.PAD] = RolePolicy(
            role=Role.PAD,
            envelope=EnvelopePolicy(
                attack_min=0.2, attack_max=0.8,
                decay_min=0.4, decay_max=0.6,
                sustain_min=0.6, sustain_max=0.8,
                release_min=0.6, release_max=3.0,
                curve=EnvelopeCurve.LINEAR,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=400, cutoff_max=1200,
                resonance_min=0.1, resonance_max=0.3,
                envelope_amount_min=0.2, envelope_amount_max=0.5,
                preferred_type=FilterType.LOW_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=1.0,  # Wide stereo
                motion_amount=0.5,  # Medium motion
                mono_compatible=False,
                reverb_amount=0.6,
                delay_amount=0.4
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.1, lfo_rate_max=2.0,
                vibrato_freq_min=4.0, vibrato_freq_max=7.0,
                vibrato_depth_min=5.0, vibrato_depth_max=15.0,
                tremolo_amount=0.3
            ),
            synthesis_preferences=[SynthesisType.SUBTRACTIVE, SynthesisType.ADDITIVE],
            tempo_scaling=True,
            key_awareness=True,
            priority=1
        )
        
        # Bass policy
        self.policies[Role.BASS] = RolePolicy(
            role=Role.BASS,
            envelope=EnvelopePolicy(
                attack_min=0.005, attack_max=0.04,
                decay_min=0.08, decay_max=0.25,
                sustain_min=0.7, sustain_max=0.9,
                release_min=0.08, release_max=0.25,
                curve=EnvelopeCurve.EXPONENTIAL,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=200, cutoff_max=800,
                resonance_min=0.4, resonance_max=0.7,
                envelope_amount_min=0.3, envelope_amount_max=0.6,
                preferred_type=FilterType.LOW_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=0.0,  # Mono
                motion_amount=0.2,  # Low motion
                mono_compatible=True,
                reverb_amount=0.2,
                delay_amount=0.1
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.5, lfo_rate_max=4.0,
                vibrato_freq_min=4.0, vibrato_freq_max=6.0,
                vibrato_depth_min=10.0, vibrato_depth_max=20.0,
                tremolo_amount=0.1
            ),
            synthesis_preferences=[SynthesisType.SUBTRACTIVE, SynthesisType.WAVETABLE],
            tempo_scaling=True,
            key_awareness=True,
            priority=2
        )
        
        # Lead policy
        self.policies[Role.LEAD] = RolePolicy(
            role=Role.LEAD,
            envelope=EnvelopePolicy(
                attack_min=0.005, attack_max=0.12,
                decay_min=0.12, decay_max=0.6,
                sustain_min=0.8, sustain_max=0.95,
                release_min=0.12, release_max=0.6,
                curve=EnvelopeCurve.EXPONENTIAL,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=800, cutoff_max=2000,
                resonance_min=0.3, resonance_max=0.6,
                envelope_amount_min=0.4, envelope_amount_max=0.7,
                preferred_type=FilterType.LOW_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=0.5,  # Medium stereo
                motion_amount=0.6,  # Medium motion
                mono_compatible=True,
                reverb_amount=0.4,
                delay_amount=0.3
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.2, lfo_rate_max=8.0,
                vibrato_freq_min=5.0, vibrato_freq_max=7.0,
                vibrato_depth_min=8.0, vibrato_depth_max=15.0,
                tremolo_amount=0.4
            ),
            synthesis_preferences=[SynthesisType.SUBTRACTIVE, SynthesisType.WAVETABLE, SynthesisType.FM],
            tempo_scaling=True,
            key_awareness=True,
            priority=3
        )
        
        # FX policy
        self.policies[Role.FX] = RolePolicy(
            role=Role.FX,
            envelope=EnvelopePolicy(
                attack_min=0.1, attack_max=0.5,
                decay_min=0.2, decay_max=1.0,
                sustain_min=0.3, sustain_max=0.7,
                release_min=0.5, release_max=2.0,
                curve=EnvelopeCurve.EXPONENTIAL,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=200, cutoff_max=8000,
                resonance_min=0.2, resonance_max=0.8,
                envelope_amount_min=0.5, envelope_amount_max=0.9,
                preferred_type=FilterType.BAND_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=1.0,  # Full stereo
                motion_amount=0.8,  # High motion
                mono_compatible=False,
                reverb_amount=0.8,
                delay_amount=0.7
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.1, lfo_rate_max=10.0,
                vibrato_freq_min=0.5, vibrato_freq_max=5.0,
                vibrato_depth_min=5.0, vibrato_depth_max=50.0,
                tremolo_amount=0.6
            ),
            synthesis_preferences=[SynthesisType.WAVETABLE, SynthesisType.MODULAR, SynthesisType.GRANULAR],
            tempo_scaling=False,
            key_awareness=False,
            priority=4
        )
        
        # Add other roles...
        self._add_texture_policy()
        self._add_arp_policy()
        self._add_drone_policy()
        self._add_rhythm_policy()
        self._add_bell_policy()
        self._add_chord_policy()
        self._add_pluck_policy()
    
    def _add_texture_policy(self):
        """Add texture role policy"""
        self.policies[Role.TEXTURE] = RolePolicy(
            role=Role.TEXTURE,
            envelope=EnvelopePolicy(
                attack_min=0.2, attack_max=1.0,
                decay_min=0.3, decay_max=1.5,
                sustain_min=0.4, sustain_max=0.8,
                release_min=1.0, release_max=5.0,
                curve=EnvelopeCurve.EXPONENTIAL,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=300, cutoff_max=2000,
                resonance_min=0.3, resonance_max=0.7,
                envelope_amount_min=0.4, envelope_amount_max=0.8,
                preferred_type=FilterType.BAND_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=1.0,
                motion_amount=0.7,
                mono_compatible=False,
                reverb_amount=0.7,
                delay_amount=0.5
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.1, lfo_rate_max=3.0,
                vibrato_freq_min=2.0, vibrato_freq_max=6.0,
                vibrato_depth_min=10.0, vibrato_depth_max=30.0,
                tremolo_amount=0.5
            ),
            synthesis_preferences=[SynthesisType.FM, SynthesisType.GRANULAR, SynthesisType.MODULAR],
            tempo_scaling=True,
            key_awareness=True,
            priority=2
        )
    
    def _add_arp_policy(self):
        """Add arpeggiator role policy"""
        self.policies[Role.ARP] = RolePolicy(
            role=Role.ARP,
            envelope=EnvelopePolicy(
                attack_min=0.005, attack_max=0.02,
                decay_min=0.05, decay_max=0.3,
                sustain_min=0.3, sustain_max=0.6,
                release_min=0.1, release_max=0.5,
                curve=EnvelopeCurve.EXPONENTIAL,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=600, cutoff_max=2000,
                resonance_min=0.2, resonance_max=0.5,
                envelope_amount_min=0.2, envelope_amount_max=0.5,
                preferred_type=FilterType.LOW_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=0.3,
                motion_amount=0.8,
                mono_compatible=True,
                reverb_amount=0.3,
                delay_amount=0.6
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=1.0, lfo_rate_max=16.0,
                vibrato_freq_min=6.0, vibrato_freq_max=12.0,
                vibrato_depth_min=5.0, vibrato_depth_max=20.0,
                tremolo_amount=0.7
            ),
            synthesis_preferences=[SynthesisType.SUBTRACTIVE, SynthesisType.WAVETABLE],
            tempo_scaling=True,
            key_awareness=True,
            priority=3
        )
    
    def _add_drone_policy(self):
        """Add drone role policy"""
        self.policies[Role.DRONE] = RolePolicy(
            role=Role.DRONE,
            envelope=EnvelopePolicy(
                attack_min=1.0, attack_max=3.0,
                decay_min=0.5, decay_max=2.0,
                sustain_min=0.8, sustain_max=1.0,
                release_min=2.0, release_max=10.0,
                curve=EnvelopeCurve.LOGARITHMIC,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=100, cutoff_max=800,
                resonance_min=0.1, resonance_max=0.3,
                envelope_amount_min=0.1, envelope_amount_max=0.3,
                preferred_type=FilterType.LOW_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=1.0,
                motion_amount=0.3,
                mono_compatible=False,
                reverb_amount=0.9,
                delay_amount=0.2
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.05, lfo_rate_max=1.0,
                vibrato_freq_min=1.0, vibrato_freq_max=4.0,
                vibrato_depth_min=2.0, vibrato_depth_max=10.0,
                tremolo_amount=0.2
            ),
            synthesis_preferences=[SynthesisType.ADDITIVE, SynthesisType.GRANULAR],
            tempo_scaling=False,
            key_awareness=True,
            priority=1
        )
    
    def _add_rhythm_policy(self):
        """Add rhythm role policy"""
        self.policies[Role.RHYTHM] = RolePolicy(
            role=Role.RHYTHM,
            envelope=EnvelopePolicy(
                attack_min=0.001, attack_max=0.01,
                decay_min=0.05, decay_max=0.2,
                sustain_min=0.1, sustain_max=0.4,
                release_min=0.05, release_max=0.3,
                curve=EnvelopeCurve.EXPONENTIAL,
                type=EnvelopeType.AD
            ),
            filter=FilterPolicy(
                cutoff_min=800, cutoff_max=4000,
                resonance_min=0.4, resonance_max=0.8,
                envelope_amount_min=0.3, envelope_amount_max=0.7,
                preferred_type=FilterType.BAND_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=0.2,
                motion_amount=0.9,
                mono_compatible=True,
                reverb_amount=0.1,
                delay_amount=0.8
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=2.0, lfo_rate_max=32.0,
                vibrato_freq_min=8.0, vibrato_freq_max=20.0,
                vibrato_depth_min=10.0, vibrato_depth_max=40.0,
                tremolo_amount=0.8
            ),
            synthesis_preferences=[SynthesisType.SUBTRACTIVE, SynthesisType.WAVETABLE],
            tempo_scaling=True,
            key_awareness=False,
            priority=4
        )
    
    def _add_bell_policy(self):
        """Add bell role policy"""
        self.policies[Role.BELL] = RolePolicy(
            role=Role.BELL,
            envelope=EnvelopePolicy(
                attack_min=0.01, attack_max=0.05,
                decay_min=0.2, decay_max=1.5,
                sustain_min=0.1, sustain_max=0.3,
                release_min=0.5, release_max=3.0,
                curve=EnvelopeCurve.LOGARITHMIC,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=1500, cutoff_max=8000,
                resonance_min=0.2, resonance_max=0.5,
                envelope_amount_min=0.1, envelope_amount_max=0.4,
                preferred_type=FilterType.HIGH_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=0.6,
                motion_amount=0.4,
                mono_compatible=True,
                reverb_amount=0.5,
                delay_amount=0.3
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.5, lfo_rate_max=4.0,
                vibrato_freq_min=3.0, vibrato_freq_max=8.0,
                vibrato_depth_min=5.0, vibrato_depth_max=15.0,
                tremolo_amount=0.3
            ),
            synthesis_preferences=[SynthesisType.ADDITIVE, SynthesisType.FM],
            tempo_scaling=True,
            key_awareness=True,
            priority=3
        )
    
    def _add_chord_policy(self):
        """Add chord role policy"""
        self.policies[Role.CHORD] = RolePolicy(
            role=Role.CHORD,
            envelope=EnvelopePolicy(
                attack_min=0.1, attack_max=0.5,
                decay_min=0.2, decay_max=0.8,
                sustain_min=0.6, sustain_max=0.9,
                release_min=0.5, release_max=2.0,
                curve=EnvelopeCurve.LINEAR,
                type=EnvelopeType.ADSR
            ),
            filter=FilterPolicy(
                cutoff_min=400, cutoff_max=1200,
                resonance_min=0.1, resonance_max=0.4,
                envelope_amount_min=0.2, envelope_amount_max=0.5,
                preferred_type=FilterType.LOW_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=0.8,
                motion_amount=0.5,
                mono_compatible=True,
                reverb_amount=0.6,
                delay_amount=0.4
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.2, lfo_rate_max=2.0,
                vibrato_freq_min=4.0, vibrato_freq_max=7.0,
                vibrato_depth_min=8.0, vibrato_depth_max=20.0,
                tremolo_amount=0.4
            ),
            synthesis_preferences=[SynthesisType.SUBTRACTIVE, SynthesisType.ADDITIVE],
            tempo_scaling=True,
            key_awareness=True,
            priority=2
        )
    
    def _add_pluck_policy(self):
        """Add pluck role policy"""
        self.policies[Role.PLUCK] = RolePolicy(
            role=Role.PLUCK,
            envelope=EnvelopePolicy(
                attack_min=0.001, attack_max=0.01,
                decay_min=0.1, decay_max=0.5,
                sustain_min=0.2, sustain_max=0.5,
                release_min=0.2, release_max=1.0,
                curve=EnvelopeCurve.EXPONENTIAL,
                type=EnvelopeType.AD
            ),
            filter=FilterPolicy(
                cutoff_min=600, cutoff_max=2000,
                resonance_min=0.2, resonance_max=0.5,
                envelope_amount_min=0.3, envelope_amount_max=0.6,
                preferred_type=FilterType.LOW_PASS
            ),
            spatial=SpatialPolicy(
                stereo_width=0.4,
                motion_amount=0.6,
                mono_compatible=True,
                reverb_amount=0.3,
                delay_amount=0.5
            ),
            modulation=ModulationPolicy(
                lfo_rate_min=0.5, lfo_rate_max=8.0,
                vibrato_freq_min=5.0, vibrato_freq_max=10.0,
                vibrato_depth_min=8.0, vibrato_depth_max=25.0,
                tremolo_amount=0.5
            ),
            synthesis_preferences=[SynthesisType.PHYSICAL_MODELING, SynthesisType.SUBTRACTIVE],
            tempo_scaling=True,
            key_awareness=True,
            priority=3
        )

if __name__ == "__main__":
    from core_models import JsonPresetParser
    
    # Test the role policy system
    parser = JsonPresetParser()
    presets = parser.parse_from_file('group.json')
    
    if presets:
        policy_manager = RolePolicyManager()
        
        # Test policy application
        test_preset = presets[0]
        print(f"Testing policy application for preset: {test_preset.name}")
        
        for role in [Role.PAD, Role.BASS, Role.LEAD, Role.FX]:
            policy = policy_manager.get_policy(role)
            adjustments = policy_manager.apply_policy(test_preset, role, tempo=120.0, key="C")
            
            print(f"\n{role.value.upper()} policy adjustments:")
            for category, params in adjustments.items():
                print(f"  {category}: {params}")