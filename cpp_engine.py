#!/usr/bin/env python3
"""
Python adapter for the C++ audio engine
Provides a clean Python interface to the pybind11-wrapped C++ code
"""

import numpy as np
from typing import Dict, List, Any, Optional
import logging

logger = logging.getLogger(__name__)

try:
    import aiaudio_cpp
    CPP_ENGINE_AVAILABLE = True
    logger.info("C++ audio engine loaded successfully")
except ImportError as e:
    CPP_ENGINE_AVAILABLE = False
    logger.warning(f"C++ audio engine not available: {e}")
    logger.warning("Install with: pip install -e .")


class CPPAudioEngineAdapter:
    """
    Adapter class that provides a clean Python interface to the C++ engine.
    Falls back to Python-only implementation if C++ engine is not available.
    """
    
    def __init__(self, fallback_to_python: bool = True):
        """
        Initialize the C++ audio engine adapter.
        
        Args:
            fallback_to_python: If True, fall back to Python implementation
                              when C++ engine is not available
        """
        self.cpp_engine = None
        self.fallback_to_python = fallback_to_python
        self.use_cpp = CPP_ENGINE_AVAILABLE
        
        if CPP_ENGINE_AVAILABLE:
            try:
                self.cpp_engine = aiaudio_cpp.CPPAudioEngine()
                logger.info("C++ audio engine initialized")
            except Exception as e:
                logger.error(f"Failed to initialize C++ engine: {e}")
                if not fallback_to_python:
                    raise
                self.use_cpp = False
        else:
            if not fallback_to_python:
                raise RuntimeError("C++ engine not available and fallback disabled")
            logger.info("Using Python-only implementation")
    
    def render_audio(self, 
                     preset: Dict[str, Any],
                     context: Optional[Dict[str, Any]] = None,
                     duration: float = 2.0) -> np.ndarray:
        """
        Render audio from a preset.
        
        Args:
            preset: Preset dictionary with role, description, parameters, etc.
            context: Musical context (tempo, key, scale, time_signature)
            duration: Audio duration in seconds
            
        Returns:
            Audio buffer as NumPy array, shape (num_samples, 2), dtype float32
            
        Raises:
            RuntimeError: If rendering fails
            ValueError: If preset is invalid
        """
        if context is None:
            context = {
                "tempo": 120.0,
                "key": 0,
                "scale": "major",
                "time_signature": [4, 4]
            }
        
        # Normalize preset format
        preset_dict = self._normalize_preset(preset)
        
        if self.use_cpp and self.cpp_engine:
            try:
                audio = self.cpp_engine.render_audio(preset_dict, context, duration)
                return audio
            except Exception as e:
                logger.error(f"C++ rendering failed: {e}")
                if not self.fallback_to_python:
                    raise
                logger.info("Falling back to Python implementation")
        
        # Fallback to Python implementation
        return self._render_audio_python(preset_dict, context, duration)
    
    def assess_quality(self,
                      audio: np.ndarray,
                      role: str,
                      context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Assess audio quality using MOO optimizer.
        
        Args:
            audio: Audio buffer, shape (num_samples, 2), dtype float32
            role: Preset role (pad, bass, lead, drum, fx)
            context: Musical context
            
        Returns:
            Quality metrics dictionary:
            {
                "overall_score": 0.85,
                "semantic_match": 0.82,
                "mix_readiness": 0.88,
                "perceptual_quality": 0.87,
                "stability": 0.95,
                "violations": [],
                "warnings": []
            }
        """
        if context is None:
            context = {
                "tempo": 120.0,
                "key": 0,
                "scale": "major"
            }
        
        if self.use_cpp and self.cpp_engine:
            try:
                return self.cpp_engine.assess_quality(audio, role, context)
            except Exception as e:
                logger.error(f"C++ quality assessment failed: {e}")
                if not self.fallback_to_python:
                    raise
        
        # Fallback to Python implementation
        return self._assess_quality_python(audio, role, context)
    
    def get_status(self) -> Dict[str, Any]:
        """
        Get system status.
        
        Returns:
            Status dictionary with initialized, loaded_presets, cpu_usage, etc.
        """
        if self.use_cpp and self.cpp_engine:
            try:
                return self.cpp_engine.get_status()
            except Exception as e:
                logger.error(f"Failed to get C++ status: {e}")
        
        return {
            "initialized": True,
            "loaded_presets": 0,
            "cpu_usage": 0.0,
            "memory_usage": 0.0,
            "active_features": ["python_fallback"],
            "engine": "python"
        }
    
    def load_preset(self, preset_path: str):
        """Load preset from file."""
        if self.use_cpp and self.cpp_engine:
            try:
                self.cpp_engine.load_preset(preset_path)
                return
            except Exception as e:
                logger.error(f"Failed to load preset in C++: {e}")
        
        logger.info(f"Loaded preset: {preset_path} (Python mode)")
    
    def get_available_presets(self) -> List[str]:
        """Get list of available presets."""
        if self.use_cpp and self.cpp_engine:
            try:
                return self.cpp_engine.get_available_presets()
            except Exception as e:
                logger.error(f"Failed to get presets from C++: {e}")
        
        return []
    
    def set_configuration(self, config: Dict[str, str]):
        """Set system configuration."""
        if self.use_cpp and self.cpp_engine:
            try:
                self.cpp_engine.set_configuration(config)
                return
            except Exception as e:
                logger.error(f"Failed to set C++ configuration: {e}")
        
        logger.info(f"Configuration set (Python mode): {config}")
    
    def _normalize_preset(self, preset: Dict[str, Any]) -> Dict[str, Any]:
        """
        Normalize preset to expected format.
        """
        normalized = {
            "prompt": preset.get("description", preset.get("name", "unknown")),
            "role": preset.get("category", preset.get("role", "unknown")).lower(),
            "constraints": {
                "max_cpu": 0.8,
                "max_latency": 10.0,
                "lufs_target": -18.0,
                "true_peak_limit": -1.0
            }
        }
        
        # Copy over any existing constraints
        if "constraints" in preset:
            normalized["constraints"].update(preset["constraints"])
        
        return normalized
    
    def _render_audio_python(self,
                            preset: Dict[str, Any],
                            context: Dict[str, Any],
                            duration: float) -> np.ndarray:
        """
        Pure Python fallback implementation.
        Generates a simple tone for testing.
        """
        sample_rate = 44100
        num_samples = int(duration * sample_rate)
        
        # Generate a simple sine wave
        t = np.linspace(0, duration, num_samples, False)
        frequency = 440.0  # A4
        
        # Simple sine wave with envelope
        audio = np.sin(2 * np.pi * frequency * t)
        
        # Apply simple ADSR envelope
        attack = 0.1
        decay = 0.2
        sustain = 0.7
        release = 0.5
        
        envelope = np.ones(num_samples)
        attack_samples = int(attack * sample_rate)
        decay_samples = int(decay * sample_rate)
        release_samples = int(release * sample_rate)
        
        # Attack
        if attack_samples > 0:
            envelope[:attack_samples] = np.linspace(0, 1, attack_samples)
        
        # Decay
        if decay_samples > 0:
            start = attack_samples
            end = min(start + decay_samples, num_samples)
            envelope[start:end] = np.linspace(1, sustain, end - start)
        
        # Sustain
        sustain_start = attack_samples + decay_samples
        sustain_end = max(0, num_samples - release_samples)
        if sustain_start < sustain_end:
            envelope[sustain_start:sustain_end] = sustain
        
        # Release
        if release_samples > 0:
            release_start = max(0, num_samples - release_samples)
            envelope[release_start:] = np.linspace(sustain, 0, num_samples - release_start)
        
        audio *= envelope
        
        # Convert to stereo
        audio_stereo = np.column_stack((audio, audio)).astype(np.float32)
        
        return audio_stereo
    
    def _assess_quality_python(self,
                              audio: np.ndarray,
                              role: str,
                              context: Dict[str, Any]) -> Dict[str, Any]:
        """
        Pure Python fallback quality assessment.
        """
        # Simple quality metrics based on audio characteristics
        rms = np.sqrt(np.mean(audio ** 2))
        peak = np.max(np.abs(audio))
        
        # Simple quality score based on RMS and peak
        quality = 0.7 if 0.1 < rms < 0.8 and peak < 1.0 else 0.5
        
        return {
            "overall_score": quality,
            "semantic_match": quality * 0.9,
            "mix_readiness": quality * 1.1,
            "perceptual_quality": quality,
            "stability": quality * 1.05,
            "violations": [],
            "warnings": [] if peak < 1.0 else ["Potential clipping detected"]
        }


# Global instance for easy access
_engine_instance: Optional[CPPAudioEngineAdapter] = None


def get_engine(fallback_to_python: bool = True) -> CPPAudioEngineAdapter:
    """
    Get or create the global C++ engine instance.
    
    Args:
        fallback_to_python: If True, fall back to Python implementation
        
    Returns:
        CPPAudioEngineAdapter instance
    """
    global _engine_instance
    if _engine_instance is None:
        _engine_instance = CPPAudioEngineAdapter(fallback_to_python)
    return _engine_instance


# Convenience functions
def render_audio(preset: Dict[str, Any],
                context: Optional[Dict[str, Any]] = None,
                duration: float = 2.0) -> np.ndarray:
    """Render audio using the global engine instance."""
    return get_engine().render_audio(preset, context, duration)


def assess_quality(audio: np.ndarray,
                  role: str,
                  context: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
    """Assess quality using the global engine instance."""
    return get_engine().assess_quality(audio, role, context)
