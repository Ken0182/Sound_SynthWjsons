#!/usr/bin/env python3
"""
C++ Engine Adapter for AI Audio Generator
Provides Python interface to the C++ audio rendering engine via pybind11
"""

import numpy as np
from typing import Dict, List, Any, Optional, Union
import json
import os

try:
    import aiaudio_python
    CPP_ENGINE_AVAILABLE = True
except ImportError:
    CPP_ENGINE_AVAILABLE = False
    print("Warning: C++ engine not available. Install with: pip install -e .")

class CPPAudioEngine:
    """Python adapter for the C++ AI Audio Generator"""
    
    def __init__(self):
        if not CPP_ENGINE_AVAILABLE:
            raise ImportError("C++ engine not available. Please build the project first.")
        
        self.engine = aiaudio_python.AIAudioGenerator()
        self.sample_rate = 44100.0
        self.channels = 2
        
    def generate_audio(self, prompt: str, role: str = "UNKNOWN", 
                      tempo: float = 120.0, key: str = "C", scale: str = "major",
                      max_cpu: float = 0.8, max_latency: float = 10.0,
                      use_semantic_search: bool = True, apply_policies: bool = True) -> np.ndarray:
        """Generate audio from text prompt using C++ engine"""
        try:
            audio_array = self.engine.generate_audio(
                prompt=prompt,
                role=role,
                tempo=tempo,
                key=key,
                scale=scale,
                max_cpu=max_cpu,
                max_latency=max_latency,
                use_semantic_search=use_semantic_search,
                apply_policies=apply_policies
            )
            return np.array(audio_array, dtype=np.float32)
        except Exception as e:
            print(f"Error generating audio: {e}")
            return np.array([], dtype=np.float32)
    
    def generate_from_preset(self, preset_params: Dict[str, Any], 
                           duration: float = 2.0, sample_rate: float = 44100.0) -> np.ndarray:
        """Generate audio from preset parameters using C++ engine"""
        try:
            # Convert preset parameters to Python dict for C++ binding
            py_params = {}
            for key, value in preset_params.items():
                if isinstance(value, (int, float, str)):
                    py_params[key] = value
                else:
                    py_params[key] = str(value)
            
            audio_array = self.engine.generate_from_preset(
                preset_params=py_params,
                duration=duration,
                sample_rate=sample_rate
            )
            return np.array(audio_array, dtype=np.float32)
        except Exception as e:
            print(f"Error generating from preset: {e}")
            return np.array([], dtype=np.float32)
    
    def get_status(self) -> Dict[str, Any]:
        """Get system status from C++ engine"""
        try:
            return dict(self.engine.get_status())
        except Exception as e:
            print(f"Error getting status: {e}")
            return {"error": str(e)}
    
    def load_preset(self, file_path: str) -> bool:
        """Load preset from file using C++ engine"""
        try:
            return self.engine.load_preset(file_path)
        except Exception as e:
            print(f"Error loading preset: {e}")
            return False
    
    def get_available_presets(self) -> List[str]:
        """Get list of available presets from C++ engine"""
        try:
            return list(self.engine.get_available_presets())
        except Exception as e:
            print(f"Error getting presets: {e}")
            return []
    
    def render_preset_to_audio(self, preset_data: Dict[str, Any], 
                             duration: float = 2.0) -> np.ndarray:
        """Render a preset to audio using C++ engine"""
        # Extract key parameters for C++ rendering
        cpp_params = {}
        
        # Map common preset parameters
        if 'frequency' in preset_data:
            cpp_params['frequency'] = float(preset_data['frequency'])
        if 'oscillator' in preset_data:
            osc = preset_data['oscillator']
            if 'frequency' in osc:
                cpp_params['frequency'] = float(osc['frequency'])
            if 'waveform' in osc:
                cpp_params['waveform'] = str(osc['waveform'])
        
        if 'envelope' in preset_data:
            env = preset_data['envelope']
            if 'attack' in env:
                cpp_params['attack'] = float(env['attack'])
            if 'decay' in env:
                cpp_params['decay'] = float(env['decay'])
            if 'sustain' in env:
                cpp_params['sustain'] = float(env['sustain'])
            if 'release' in env:
                cpp_params['release'] = float(env['release'])
        
        if 'filter' in preset_data:
            filt = preset_data['filter']
            if 'cutoff' in filt:
                cpp_params['cutoff'] = float(filt['cutoff'])
            if 'resonance' in filt:
                cpp_params['resonance'] = float(filt['resonance'])
        
        # Add any other numeric parameters
        for key, value in preset_data.items():
            if isinstance(value, (int, float)) and key not in cpp_params:
                cpp_params[key] = float(value)
        
        return self.generate_from_preset(cpp_params, duration, self.sample_rate)

class HybridAudioInterface:
    """Hybrid audio interface that combines Python and C++ engines"""
    
    def __init__(self, use_cpp_engine: bool = True):
        self.use_cpp_engine = use_cpp_engine and CPP_ENGINE_AVAILABLE
        self.cpp_engine = None
        
        if self.use_cpp_engine:
            try:
                self.cpp_engine = CPPAudioEngine()
                print("C++ engine initialized successfully")
            except Exception as e:
                print(f"Failed to initialize C++ engine: {e}")
                self.use_cpp_engine = False
        
        if not self.use_cpp_engine:
            print("Using Python-only audio generation")
    
    def generate_audio(self, prompt: str, role: str = "UNKNOWN", 
                      tempo: float = 120.0, key: str = "C", scale: str = "major",
                      max_cpu: float = 0.8, max_latency: float = 10.0) -> np.ndarray:
        """Generate audio using the best available engine"""
        if self.use_cpp_engine and self.cpp_engine:
            return self.cpp_engine.generate_audio(
                prompt, role, tempo, key, scale, max_cpu, max_latency
            )
        else:
            # Fallback to Python-only generation
            return self._generate_python_audio(prompt, role, tempo, key, scale)
    
    def generate_from_preset(self, preset_data: Dict[str, Any], 
                           duration: float = 2.0) -> np.ndarray:
        """Generate audio from preset using the best available engine"""
        if self.use_cpp_engine and self.cpp_engine:
            return self.cpp_engine.render_preset_to_audio(preset_data, duration)
        else:
            # Fallback to Python-only generation
            return self._generate_python_preset_audio(preset_data, duration)
    
    def _generate_python_audio(self, prompt: str, role: str, tempo: float, 
                              key: str, scale: str) -> np.ndarray:
        """Fallback Python-only audio generation"""
        # Simple sine wave generation as fallback
        duration = 2.0
        sample_rate = 44100
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Generate a simple tone based on role
        if role == "BASS":
            frequency = 80.0
        elif role == "LEAD":
            frequency = 440.0
        elif role == "PAD":
            frequency = 220.0
        else:
            frequency = 440.0
        
        audio = np.sin(2 * np.pi * frequency * t)
        
        # Apply simple envelope
        attack_samples = int(0.1 * sample_rate)
        release_samples = int(0.5 * sample_rate)
        
        if len(audio) > attack_samples:
            audio[:attack_samples] *= np.linspace(0, 1, attack_samples)
        if len(audio) > release_samples:
            audio[-release_samples:] *= np.linspace(1, 0, release_samples)
        
        return audio.astype(np.float32)
    
    def _generate_python_preset_audio(self, preset_data: Dict[str, Any], 
                                    duration: float) -> np.ndarray:
        """Fallback Python-only preset audio generation"""
        sample_rate = 44100
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Extract frequency
        frequency = 440.0
        if 'frequency' in preset_data:
            frequency = float(preset_data['frequency'])
        elif 'oscillator' in preset_data and 'frequency' in preset_data['oscillator']:
            frequency = float(preset_data['oscillator']['frequency'])
        
        # Generate audio
        audio = np.sin(2 * np.pi * frequency * t)
        
        # Apply envelope if available
        if 'envelope' in preset_data:
            env = preset_data['envelope']
            attack = float(env.get('attack', 0.1))
            release = float(env.get('release', 0.5))
            
            attack_samples = int(attack * sample_rate)
            release_samples = int(release * sample_rate)
            
            if len(audio) > attack_samples:
                audio[:attack_samples] *= np.linspace(0, 1, attack_samples)
            if len(audio) > release_samples:
                audio[-release_samples:] *= np.linspace(1, 0, release_samples)
        
        return audio.astype(np.float32)
    
    def get_status(self) -> Dict[str, Any]:
        """Get system status"""
        if self.use_cpp_engine and self.cpp_engine:
            return self.cpp_engine.get_status()
        else:
            return {
                "engine": "python_only",
                "cpp_available": CPP_ENGINE_AVAILABLE,
                "status": "ready"
            }

def create_hybrid_interface(use_cpp: bool = True) -> HybridAudioInterface:
    """Create a hybrid audio interface"""
    return HybridAudioInterface(use_cpp)

def test_cpp_engine():
    """Test the C++ engine functionality"""
    if not CPP_ENGINE_AVAILABLE:
        print("C++ engine not available")
        return False
    
    try:
        engine = CPPAudioEngine()
        print("C++ engine created successfully")
        
        # Test status
        status = engine.get_status()
        print(f"Status: {status}")
        
        # Test audio generation
        audio = engine.generate_audio("test sound", "LEAD", 120.0)
        print(f"Generated audio: {len(audio)} samples")
        
        return True
    except Exception as e:
        print(f"Error testing C++ engine: {e}")
        return False

if __name__ == "__main__":
    # Test the C++ engine
    test_cpp_engine()