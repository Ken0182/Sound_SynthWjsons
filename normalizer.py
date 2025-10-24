"""
Unit Normalization & Validation System
Implements Step 3: Unit normalization & validation rules
"""

from typing import Dict, List, Tuple, Union, Any
from dataclasses import dataclass
from core_models import JsonPreset, NormalizedPreset, SynthesisType, OscillatorType, FilterType, EnvelopeType, EnvelopeCurve, Role
import math
import re

class UnitNormalizer:
    """Normalizes all units to SI + canonical ranges with safety"""
    
    def __init__(self):
        self.role_defaults = self._get_role_defaults()
        self.safety_limits = self._get_safety_limits()
    
    def normalize(self, preset: JsonPreset) -> NormalizedPreset:
        """Normalize preset to SI units and canonical ranges"""
        issues = []
        
        # Normalize envelope
        normalized_envelope = self._normalize_envelope(preset.envelope, issues)
        
        # Normalize filter
        normalized_filter = self._normalize_filter(preset.filter, issues)
        
        # Normalize oscillator
        normalized_oscillator = self._normalize_oscillator(preset.oscillator, issues)
        
        # Normalize effects
        normalized_fx = self._normalize_effects(preset.fx, issues)
        
        # Determine role from preset characteristics
        role = self._determine_role(preset, issues)
        
        # Apply role-based defaults for missing fields
        self._apply_role_defaults(normalized_envelope, normalized_filter, normalized_oscillator, role, issues)
        
        # Validate safety constraints
        self._validate_safety_constraints(normalized_envelope, normalized_filter, normalized_fx, issues)
        
        return NormalizedPreset(
            name=preset.name,
            synthesis_type=preset.synthesis_type,
            oscillator=normalized_oscillator,
            envelope=normalized_envelope,
            filter=normalized_filter,
            fx=normalized_fx,
            sound_characteristics=preset.sound_characteristics,
            topological_metadata=preset.topological_metadata,
            ai_control=preset.ai_control,
            role=role,
            metadata=preset.metadata,
            validation_issues=issues
        )
    
    def _normalize_envelope(self, envelope, issues: List[str]):
        """Normalize envelope parameters to seconds and [0,1] ranges"""
        from core_models import EnvelopeConfig
        
        # Normalize attack time
        attack = self._normalize_time(envelope.attack, "envelope.attack", issues)
        
        # Normalize decay time
        decay = self._normalize_time(envelope.decay, "envelope.decay", issues)
        
        # Normalize sustain level
        sustain = self._normalize_percentage(envelope.sustain, "envelope.sustain", issues)
        
        # Normalize release time
        release = self._normalize_time(envelope.release, "envelope.release", issues)
        
        # Normalize hold time if present
        hold = 0.0
        if hasattr(envelope, 'hold') and envelope.hold:
            hold = self._normalize_time(envelope.hold, "envelope.hold", issues)
        
        # Normalize delay time if present
        delay = 0.0
        if hasattr(envelope, 'delay') and envelope.delay:
            delay = self._normalize_time(envelope.delay, "envelope.delay", issues)
        
        return EnvelopeConfig(
            type=envelope.type,
            attack=attack,
            decay=decay,
            sustain=sustain,
            release=release,
            hold=hold,
            delay=delay,
            curve=envelope.curve
        )
    
    def _normalize_filter(self, filter_config, issues: List[str]):
        """Normalize filter parameters to Hz and [0,1] ranges"""
        from core_models import FilterConfig
        
        # Normalize cutoff frequency
        cutoff = self._normalize_frequency(filter_config.cutoff, "filter.cutoff", issues)
        
        # Normalize resonance
        resonance = self._normalize_percentage(filter_config.resonance, "filter.resonance", issues)
        
        # Normalize envelope amount
        env_amount = self._normalize_percentage(filter_config.envelope_amount, "filter.envelope_amount", issues)
        
        return FilterConfig(
            type=filter_config.type,
            cutoff=cutoff,
            resonance=resonance,
            envelope_amount=env_amount,
            slope=filter_config.slope
        )
    
    def _normalize_oscillator(self, oscillator, issues: List[str]):
        """Normalize oscillator parameters"""
        from core_models import OscillatorConfig
        
        # Normalize detune (cents to ratio)
        detune = self._normalize_detune(oscillator.detune, "oscillator.detune", issues)
        
        # Normalize mix ratios to sum to 1.0
        mix_ratios = self._normalize_mix_ratios(oscillator.mix_ratios, "oscillator.mix_ratios", issues)
        
        # Normalize modulation index if present
        mod_index = oscillator.modulation_index
        if mod_index is not None:
            mod_index = self._clamp_value(mod_index, 0.0, 10.0, "oscillator.modulation_index", issues)
        
        # Normalize carrier ratio if present
        carrier_ratio = oscillator.carrier_ratio
        if carrier_ratio is not None:
            carrier_ratio = self._clamp_value(carrier_ratio, 0.1, 10.0, "oscillator.carrier_ratio", issues)
        
        return OscillatorConfig(
            types=oscillator.types,
            mix_ratios=mix_ratios,
            detune=detune,
            modulation_index=mod_index,
            carrier_ratio=carrier_ratio,
            harmonics=oscillator.harmonics,
            morph_rate=oscillator.morph_rate,
            table_index=oscillator.table_index,
            grain_density=oscillator.grain_density,
            grain_size=oscillator.grain_size,
            pluck_position=oscillator.pluck_position,
            blend_mode=oscillator.blend_mode
        )
    
    def _normalize_effects(self, effects, issues: List[str]):
        """Normalize effect parameters"""
        from core_models import EffectConfig
        normalized_effects = []
        
        for i, effect in enumerate(effects):
            # Normalize mix
            mix = self._normalize_percentage(effect.mix, f"fx[{i}].mix", issues)
            
            # Normalize effect-specific parameters
            normalized_params = {"type": effect.type, "mix": mix}
            
            if effect.feedback is not None:
                normalized_params["feedback"] = self._clamp_value(
                    self._normalize_percentage(effect.feedback, f"fx[{i}].feedback", issues),
                    0.0, 0.85, f"fx[{i}].feedback", issues
                )
            
            if effect.time is not None:
                normalized_params["time"] = self._normalize_time(effect.time, f"fx[{i}].time", issues)
            
            if effect.gain is not None:
                # Convert gain to dB and clamp
                gain_db = self._normalize_gain(effect.gain, f"fx[{i}].gain", issues)
                normalized_params["gain"] = self._clamp_value(gain_db, -60.0, 12.0, f"fx[{i}].gain", issues)
            
            if effect.decay is not None:
                normalized_params["decay"] = self._normalize_time(effect.decay, f"fx[{i}].decay", issues)
            
            if effect.wet is not None:
                normalized_params["wet"] = self._normalize_percentage(effect.wet, f"fx[{i}].wet", issues)
            
            if effect.rate is not None:
                normalized_params["rate"] = self._normalize_frequency(effect.rate, f"fx[{i}].rate", issues)
            
            if effect.depth is not None:
                normalized_params["depth"] = self._normalize_percentage(effect.depth, f"fx[{i}].depth", issues)
            
            if effect.frequency is not None:
                normalized_params["frequency"] = self._normalize_frequency(effect.frequency, f"fx[{i}].frequency", issues)
            
            if effect.density is not None:
                normalized_params["density"] = self._normalize_percentage(effect.density, f"fx[{i}].density", issues)
            
            if effect.threshold is not None:
                normalized_params["threshold"] = self._normalize_gain(effect.threshold, f"fx[{i}].threshold", issues)
            
            if effect.ratio is not None:
                normalized_params["ratio"] = self._clamp_value(
                    effect.ratio if isinstance(effect.ratio, float) else effect.ratio[0],
                    1.0, 20.0, f"fx[{i}].ratio", issues
                )
            
            # Create normalized effect
            normalized_effect = EffectConfig(
                type=effect.type,
                mix=normalized_params["mix"],
                feedback=normalized_params.get("feedback"),
                time=normalized_params.get("time"),
                gain=normalized_params.get("gain"),
                amount=normalized_params.get("amount"),
                decay=normalized_params.get("decay"),
                wet=normalized_params.get("wet"),
                rate=normalized_params.get("rate"),
                depth=normalized_params.get("depth"),
                frequency=normalized_params.get("frequency"),
                density=normalized_params.get("density"),
                threshold=normalized_params.get("threshold"),
                ratio=normalized_params.get("ratio")
            )
            normalized_effects.append(normalized_effect)
        
        return normalized_effects
    
    def _normalize_time(self, time_value, param_name: str, issues: List[str]) -> float:
        """Convert time to seconds"""
        if isinstance(time_value, (int, float)):
            # Assume milliseconds if < 10, seconds if >= 10
            if time_value < 10:
                return float(time_value) / 1000.0
            else:
                return float(time_value)
        elif isinstance(time_value, str):
            if time_value.endswith('ms'):
                return float(time_value[:-2]) / 1000.0
            elif time_value.endswith('s'):
                return float(time_value[:-1])
            elif '-' in time_value:
                # Handle range format like "80-250"
                try:
                    parts = time_value.split('-')
                    if len(parts) == 2:
                        min_val = float(parts[0])
                        max_val = float(parts[1])
                        return (min_val + max_val) / 2.0 / 1000.0  # Average in seconds
                    else:
                        return float(time_value) / 1000.0
                except ValueError:
                    issues.append(f"Invalid time range format for {param_name}: {time_value}")
                    return 0.0
            else:
                # Try to parse as number
                try:
                    val = float(time_value)
                    return val / 1000.0 if val < 10 else val
                except ValueError:
                    issues.append(f"Invalid time format for {param_name}: {time_value}")
                    return 0.0
        elif isinstance(time_value, list):
            # Use average of range
            return (time_value[0] + time_value[1]) / 2.0 / 1000.0
        else:
            issues.append(f"Invalid time value for {param_name}: {time_value}")
            return 0.0
    
    def _normalize_frequency(self, freq_value, param_name: str, issues: List[str]) -> float:
        """Convert frequency to Hz"""
        if isinstance(freq_value, (int, float)):
            return float(freq_value)
        elif isinstance(freq_value, str):
            if freq_value.endswith('Hz'):
                return float(freq_value[:-2])
            elif freq_value.endswith('kHz'):
                return float(freq_value[:-3]) * 1000.0
            else:
                try:
                    return float(freq_value)
                except ValueError:
                    issues.append(f"Invalid frequency format for {param_name}: {freq_value}")
                    return 1000.0
        elif isinstance(freq_value, list):
            return (freq_value[0] + freq_value[1]) / 2.0
        else:
            issues.append(f"Invalid frequency value for {param_name}: {freq_value}")
            return 1000.0
    
    def _normalize_percentage(self, value, param_name: str, issues: List[str]) -> float:
        """Convert percentage to [0,1] range"""
        if isinstance(value, (int, float)):
            if value > 1.0:
                # Assume percentage
                return float(value) / 100.0
            else:
                return float(value)
        elif isinstance(value, list):
            # Use average of range
            avg = (value[0] + value[1]) / 2.0
            return avg / 100.0 if avg > 1.0 else avg
        else:
            issues.append(f"Invalid percentage value for {param_name}: {value}")
            return 0.5
    
    def _normalize_detune(self, detune_value, param_name: str, issues: List[str]) -> float:
        """Convert detune from cents to frequency ratio"""
        if isinstance(detune_value, (int, float)):
            # Convert cents to ratio: ratio = 2^(cents/1200)
            return 2.0 ** (detune_value / 1200.0)
        elif isinstance(detune_value, list):
            avg = (detune_value[0] + detune_value[1]) / 2.0
            return 2.0 ** (avg / 1200.0)
        else:
            issues.append(f"Invalid detune value for {param_name}: {detune_value}")
            return 1.0
    
    def _normalize_mix_ratios(self, ratios, param_name: str, issues: List[str]) -> List[float]:
        """Normalize mix ratios to sum to 1.0"""
        if not ratios:
            return [1.0]
        
        # Convert to floats
        float_ratios = []
        for i, ratio in enumerate(ratios):
            if isinstance(ratio, (int, float)):
                float_ratios.append(float(ratio))
            elif isinstance(ratio, list):
                float_ratios.append((ratio[0] + ratio[1]) / 2.0)
            else:
                issues.append(f"Invalid mix ratio at index {i} for {param_name}: {ratio}")
                float_ratios.append(1.0)
        
        # Normalize to sum to 1.0
        total = sum(float_ratios)
        if total > 0:
            return [r / total for r in float_ratios]
        else:
            return [1.0] * len(float_ratios)
    
    def _normalize_gain(self, gain_value, param_name: str, issues: List[str]) -> float:
        """Convert gain to dB"""
        if isinstance(gain_value, (int, float)):
            if gain_value > 0:
                return 20.0 * math.log10(gain_value)
            else:
                return -60.0  # Very low gain
        elif isinstance(gain_value, list):
            avg = (gain_value[0] + gain_value[1]) / 2.0
            return 20.0 * math.log10(avg) if avg > 0 else -60.0
        else:
            issues.append(f"Invalid gain value for {param_name}: {gain_value}")
            return 0.0
    
    def _clamp_value(self, value: float, min_val: float, max_val: float, param_name: str, issues: List[str]) -> float:
        """Clamp value to range and warn if clamping occurred"""
        if value < min_val:
            issues.append(f"Clamped {param_name} from {value} to {min_val}")
            return min_val
        elif value > max_val:
            issues.append(f"Clamped {param_name} from {value} to {max_val}")
            return max_val
        else:
            return value
    
    def _determine_role(self, preset: JsonPreset, issues: List[str]) -> Role:
        """Determine role from preset characteristics"""
        # Check if role is explicitly set
        if preset.role:
            return preset.role
        
        # Infer from name patterns
        name_lower = preset.name.lower()
        if any(keyword in name_lower for keyword in ['pad', 'warm', 'calm', 'ambient']):
            return Role.PAD
        elif any(keyword in name_lower for keyword in ['bass', 'punchy', 'driving']):
            return Role.BASS
        elif any(keyword in name_lower for keyword in ['lead', 'bright', 'energetic', 'supersaw']):
            return Role.LEAD
        elif any(keyword in name_lower for keyword in ['fx', 'chaotic', 'experimental']):
            return Role.FX
        elif any(keyword in name_lower for keyword in ['texture', 'evolving', 'tense']):
            return Role.TEXTURE
        elif any(keyword in name_lower for keyword in ['arp', 'plucky', 'rhythmic']):
            return Role.ARP
        elif any(keyword in name_lower for keyword in ['drone', 'airy', 'ambient']):
            return Role.DRONE
        elif any(keyword in name_lower for keyword in ['rhythm', 'crunchy', 'aggressive']):
            return Role.RHYTHM
        elif any(keyword in name_lower for keyword in ['bell', 'glassy', 'clear']):
            return Role.BELL
        elif any(keyword in name_lower for keyword in ['chord', 'soft', 'lush']):
            return Role.CHORD
        elif any(keyword in name_lower for keyword in ['pluck', 'karplus']):
            return Role.PLUCK
        else:
            # Default to pad for unknown
            return Role.PAD
    
    def _apply_role_defaults(self, envelope, filter_config, oscillator, role: Role, issues: List[str]):
        """Apply role-based defaults for missing fields"""
        if role in self.role_defaults:
            defaults = self.role_defaults[role]
            
            # Apply envelope defaults
            if 'env' in defaults:
                env_defaults = defaults['env']
                if envelope.attack < 0.001:  # Very small attack
                    envelope.attack = self._normalize_time(env_defaults.get('atk', '50ms'), "role_default.attack", issues)
                if envelope.release < 0.001:  # Very small release
                    envelope.release = self._normalize_time(env_defaults.get('rel', '500ms'), "role_default.release", issues)
    
    def _validate_safety_constraints(self, envelope, filter_config, effects, issues: List[str]):
        """Validate safety constraints"""
        # Check filter cutoff range
        if filter_config.cutoff < 20 or filter_config.cutoff > 20000:
            issues.append(f"Filter cutoff {filter_config.cutoff}Hz outside safe range [20, 20000]")
        
        # Check resonance
        if filter_config.resonance > 0.9:
            issues.append(f"Filter resonance {filter_config.resonance} exceeds safe maximum 0.9")
        
        # Check effect feedback
        for i, effect in enumerate(effects):
            if effect.feedback is not None and effect.feedback > 0.85:
                issues.append(f"Effect {i} feedback {effect.feedback} exceeds safe maximum 0.85")
            
            if effect.gain is not None and effect.gain > 12.0:
                issues.append(f"Effect {i} gain {effect.gain}dB exceeds safe maximum 12dB")
    
    def _get_role_defaults(self) -> Dict:
        """Get role-based default parameters"""
        return {
            Role.PAD: {
                'env': {'atk': '200-800ms', 'rel': '600-3000ms'},
                'stereo': 'wide',
                'motion': 'medium',
                'mono': False
            },
            Role.BASS: {
                'env': {'atk': '5-40ms', 'rel': '80-250ms'},
                'stereo': 'mono',
                'motion': 'low',
                'mono': True
            },
            Role.LEAD: {
                'env': {'atk': '5-120ms', 'rel': '120-600ms'},
                'stereo': 'medium',
                'motion': 'medium',
                'vibrato': '5-7Hz'
            },
            Role.FX: {
                'time_q': ['1/8', '1/4', '3/8'],
                'feedback_max': 0.7
            }
        }
    
    def _get_safety_limits(self) -> Dict:
        """Get safety parameter limits"""
        return {
            'feedback_max': 0.85,
            'cutoff_min': 20,
            'cutoff_max': 20000,
            'resonance_max': 0.9,
            'drive_max_db': 12.0
        }

if __name__ == "__main__":
    from core_models import JsonPresetParser
    
    # Test the normalizer
    parser = JsonPresetParser()
    presets = parser.parse_from_file('group.json')
    
    if presets:
        normalizer = UnitNormalizer()
        normalized = normalizer.normalize(presets[0])
        
        print(f"Normalized preset: {normalized.name}")
        print(f"Role: {normalized.role}")
        print(f"Envelope attack: {normalized.envelope.attack:.3f}s")
        print(f"Filter cutoff: {normalized.filter.cutoff:.1f}Hz")
        print(f"Filter resonance: {normalized.filter.resonance:.3f}")
        
        if normalized.validation_issues:
            print("\nValidation issues:")
            for issue in normalized.validation_issues:
                print(f"  - {issue}")
        else:
            print("\nNo validation issues found")