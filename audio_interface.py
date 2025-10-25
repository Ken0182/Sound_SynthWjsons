#!/usr/bin/env python3
"""
Audio Interface for JSON Sound Presets
Provides real-time audio playback from JSON datasets with a clean API
"""

import json
import os
import sys
import argparse
import threading
import time
from typing import Dict, List, Any, Optional, Callable
from dataclasses import dataclass
from enum import Enum
import numpy as np

try:
    import pygame
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False
    print("Warning: pygame not available. Install with: pip install pygame")

try:
    import sounddevice as sd
    import soundfile as sf
    SOUNDDEVICE_AVAILABLE = True
except ImportError:
    SOUNDDEVICE_AVAILABLE = False
    print("Warning: sounddevice not available. Install with: pip install sounddevice soundfile")

@dataclass
class SoundPreset:
    """Represents a sound preset from JSON data"""
    name: str
    category: str
    description: str
    parameters: Dict[str, Any]
    audio_data: Optional[np.ndarray] = None
    sample_rate: int = 44100
    duration: float = 0.0
    
class AudioEngine(Enum):
    PYGAME = "pygame"
    SOUNDDEVICE = "sounddevice"
    MOCK = "mock"

class AudioInterface:
    """Main audio interface for playing sound presets"""
    
    def __init__(self, engine: AudioEngine = None):
        self.engine = engine or self._detect_best_engine()
        self.presets: Dict[str, SoundPreset] = {}
        self.playing_sounds: Dict[str, Any] = {}
        self.sample_rate = 44100
        self.channels = 2
        self._setup_audio_engine()
        
    def _detect_best_engine(self) -> AudioEngine:
        """Detect the best available audio engine"""
        if SOUNDDEVICE_AVAILABLE:
            return AudioEngine.SOUNDDEVICE
        elif PYGAME_AVAILABLE:
            return AudioEngine.PYGAME
        else:
            return AudioEngine.MOCK
            
    def _setup_audio_engine(self):
        """Setup the selected audio engine"""
        if self.engine == AudioEngine.PYGAME and PYGAME_AVAILABLE:
            pygame.mixer.init(frequency=self.sample_rate, size=-16, channels=self.channels)
            pygame.mixer.set_num_channels(32)  # Allow multiple simultaneous sounds
        elif self.engine == AudioEngine.SOUNDDEVICE and SOUNDDEVICE_AVAILABLE:
            # sounddevice is ready to use
            pass
        elif self.engine == AudioEngine.MOCK:
            print("Using mock audio engine - no actual audio will be played")
            
    def load_json_datasets(self, json_files: List[str]) -> int:
        """Load sound presets from JSON files"""
        loaded_count = 0
        
        for json_file in json_files:
            if not os.path.exists(json_file):
                print(f"Warning: JSON file not found: {json_file}")
                continue
                
            try:
                with open(json_file, 'r') as f:
                    data = json.load(f)
                    
                # Extract presets based on JSON structure
                presets = self._extract_presets_from_json(data, json_file)
                
                for preset in presets:
                    self.presets[preset.name] = preset
                    loaded_count += 1
                    
                print(f"Loaded {len(presets)} presets from {json_file}")
                
            except Exception as e:
                print(f"Error loading {json_file}: {e}")
                
        return loaded_count
        
    def _extract_presets_from_json(self, data: Dict[str, Any], filename: str) -> List[SoundPreset]:
        """Extract sound presets from JSON data structure"""
        presets = []
        category = os.path.splitext(os.path.basename(filename))[0]
        
        # Handle different JSON structures
        if 'instruments' in data:
            # Electronic track format
            for name, params in data['instruments'].items():
                preset = SoundPreset(
                    name=name,
                    category=category,
                    description=params.get('description', f"{category} preset"),
                    parameters=params
                )
                presets.append(preset)
                
        elif 'groups' in data:
            # Group format
            for name, params in data['groups'].items():
                preset = SoundPreset(
                    name=name,
                    category=category,
                    description=params.get('description', f"{category} group preset"),
                    parameters=params
                )
                presets.append(preset)
                
        elif 'presets' in data:
            # Direct presets format
            for name, params in data['presets'].items():
                preset = SoundPreset(
                    name=name,
                    category=category,
                    description=params.get('description', f"{category} preset"),
                    parameters=params
                )
                presets.append(preset)
                
        else:
            # Try to find any dictionary with sound-like parameters
            for key, value in data.items():
                if isinstance(value, dict) and self._looks_like_sound_preset(value):
                    preset = SoundPreset(
                        name=key,
                        category=category,
                        description=f"Sound preset from {category}",
                        parameters=value
                    )
                    presets.append(preset)
                    
        return presets
        
    def _looks_like_sound_preset(self, data: Dict[str, Any]) -> bool:
        """Check if data looks like a sound preset"""
        sound_indicators = ['oscillator', 'envelope', 'adsr', 'frequency', 'amplitude', 'filter']
        return any(indicator in str(data).lower() for indicator in sound_indicators)
        
    def generate_audio_from_preset(self, preset_name: str, duration: float = 2.0) -> Optional[np.ndarray]:
        """Generate audio data from a preset's parameters"""
        if preset_name not in self.presets:
            return None
            
        preset = self.presets[preset_name]
        
        # Generate a simple tone based on preset parameters
        t = np.linspace(0, duration, int(self.sample_rate * duration), False)
        
        # Extract frequency from parameters
        frequency = self._extract_frequency(preset.parameters)
        
        # Generate base tone
        audio = np.sin(2 * np.pi * frequency * t)
        
        # Apply envelope if available
        envelope = self._extract_envelope(preset.parameters, len(audio))
        audio *= envelope
        
        # Apply simple effects
        audio = self._apply_simple_effects(audio, preset.parameters)
        
        # Convert to stereo
        if self.channels == 2:
            audio = np.column_stack((audio, audio))
            
        return audio.astype(np.float32)
        
    def _extract_frequency(self, params: Dict[str, Any]) -> float:
        """Extract frequency from preset parameters"""
        # Look for frequency in various parameter names
        freq_keys = ['frequency', 'freq', 'pitch', 'note_frequency', 'base_frequency']
        
        for key in freq_keys:
            if key in params:
                freq = params[key]
                if isinstance(freq, (int, float)):
                    return float(freq)
                elif isinstance(freq, str) and 'hz' in freq.lower():
                    return float(freq.lower().replace('hz', '').strip())
                    
        # Default to A4 (440 Hz)
        return 440.0
        
    def _extract_envelope(self, params: Dict[str, Any], length: int) -> np.ndarray:
        """Extract ADSR envelope from preset parameters"""
        # Look for ADSR parameters
        adsr = params.get('adsr', {})
        if not adsr:
            # Look for envelope parameters
            envelope = params.get('envelope', {})
            if envelope:
                adsr = envelope
                
        if not adsr:
            # Default envelope
            return np.ones(length)
            
        # Extract ADSR values
        attack_time = self._parse_time(adsr.get('attack', '0.1s'))
        decay_time = self._parse_time(adsr.get('decay', '0.1s'))
        sustain_level = float(adsr.get('sustain', 0.7))
        release_time = self._parse_time(adsr.get('release', '0.5s'))
        
        # Generate envelope
        attack_samples = int(attack_time * self.sample_rate)
        decay_samples = int(decay_time * self.sample_rate)
        release_samples = int(release_time * self.sample_rate)
        
        envelope = np.ones(length)
        
        # Attack
        if attack_samples > 0:
            envelope[:attack_samples] = np.linspace(0, 1, attack_samples)
            
        # Decay
        if decay_samples > 0:
            start_idx = attack_samples
            end_idx = min(start_idx + decay_samples, length)
            envelope[start_idx:end_idx] = np.linspace(1, sustain_level, end_idx - start_idx)
            
        # Sustain
        sustain_start = attack_samples + decay_samples
        sustain_end = max(0, length - release_samples)
        if sustain_start < sustain_end:
            envelope[sustain_start:sustain_end] = sustain_level
            
        # Release
        if release_samples > 0:
            release_start = max(0, length - release_samples)
            envelope[release_start:] = np.linspace(sustain_level, 0, length - release_start)
            
        return envelope
        
    def _parse_time(self, time_str: str) -> float:
        """Parse time string to seconds"""
        if isinstance(time_str, (int, float)):
            return float(time_str)
            
        time_str = str(time_str).lower()
        if 'ms' in time_str:
            return float(time_str.replace('ms', '')) / 1000.0
        elif 's' in time_str:
            return float(time_str.replace('s', ''))
        else:
            return float(time_str)
            
    def _apply_simple_effects(self, audio: np.ndarray, params: Dict[str, Any]) -> np.ndarray:
        """Apply simple audio effects based on preset parameters"""
        # Look for effects
        effects = params.get('effects', [])
        if not effects:
            return audio
            
        for effect in effects:
            if isinstance(effect, dict):
                effect_type = effect.get('type', '').lower()
                amount = effect.get('amount', 0.0)
                
                if effect_type == 'reverb' and amount > 0:
                    # Simple reverb simulation
                    audio = self._apply_simple_reverb(audio, amount)
                elif effect_type == 'distortion' and amount > 0:
                    # Simple distortion
                    audio = np.tanh(audio * (1 + amount * 3))
                elif effect_type == 'lowpass' and amount > 0:
                    # Simple lowpass filter
                    audio = self._apply_simple_lowpass(audio, amount)
                    
        return audio
        
    def _apply_simple_reverb(self, audio: np.ndarray, amount: float) -> np.ndarray:
        """Apply simple reverb effect"""
        delay_samples = int(0.1 * self.sample_rate)  # 100ms delay
        if len(audio) > delay_samples:
            delayed = np.zeros_like(audio)
            delayed[delay_samples:] = audio[:-delay_samples] * amount
            audio = audio + delayed
        return audio
        
    def _apply_simple_lowpass(self, audio: np.ndarray, amount: float) -> np.ndarray:
        """Apply simple lowpass filter"""
        # Simple moving average filter
        window_size = max(1, int(amount * 10))
        if window_size > 1:
            kernel = np.ones(window_size) / window_size
            audio = np.convolve(audio, kernel, mode='same')
        return audio
        
    def play_preset(self, preset_name: str, duration: float = 2.0) -> bool:
        """Play a sound preset"""
        if preset_name not in self.presets:
            print(f"Preset not found: {preset_name}")
            return False
            
        # Stop if already playing
        if preset_name in self.playing_sounds:
            self.stop_preset(preset_name)
            
        # Generate audio
        audio_data = self.generate_audio_from_preset(preset_name, duration)
        if audio_data is None:
            return False
            
        # Play audio based on engine
        if self.engine == AudioEngine.PYGAME and PYGAME_AVAILABLE:
            return self._play_with_pygame(preset_name, audio_data)
        elif self.engine == AudioEngine.SOUNDDEVICE and SOUNDDEVICE_AVAILABLE:
            return self._play_with_sounddevice(preset_name, audio_data)
        else:
            print(f"Playing preset: {preset_name} (mock mode)")
            return True
            
    def _play_with_pygame(self, preset_name: str, audio_data: np.ndarray) -> bool:
        """Play audio using pygame"""
        try:
            # Convert to 16-bit for pygame
            audio_16bit = (audio_data * 32767).astype(np.int16)
            
            # Create pygame sound
            sound = pygame.sndarray.make_sound(audio_16bit)
            sound.play()
            
            self.playing_sounds[preset_name] = sound
            return True
        except Exception as e:
            print(f"Error playing with pygame: {e}")
            return False
            
    def _play_with_sounddevice(self, preset_name: str, audio_data: np.ndarray) -> bool:
        """Play audio using sounddevice"""
        try:
            # Play audio in a separate thread
            def play_audio():
                sd.play(audio_data, samplerate=self.sample_rate)
                sd.wait()  # Wait for playback to finish
                if preset_name in self.playing_sounds:
                    del self.playing_sounds[preset_name]
                    
            thread = threading.Thread(target=play_audio)
            thread.daemon = True
            thread.start()
            
            self.playing_sounds[preset_name] = thread
            return True
        except Exception as e:
            print(f"Error playing with sounddevice: {e}")
            return False
            
    def stop_preset(self, preset_name: str) -> bool:
        """Stop playing a preset"""
        if preset_name not in self.playing_sounds:
            return False
            
        if self.engine == AudioEngine.PYGAME and PYGAME_AVAILABLE:
            sound = self.playing_sounds[preset_name]
            sound.stop()
        elif self.engine == AudioEngine.SOUNDDEVICE and SOUNDDEVICE_AVAILABLE:
            # For sounddevice, we can't easily stop individual sounds
            # The thread will finish naturally
            pass
            
        del self.playing_sounds[preset_name]
        return True
        
    def stop_all(self):
        """Stop all playing sounds"""
        for preset_name in list(self.playing_sounds.keys()):
            self.stop_preset(preset_name)
            
    def get_preset_info(self, preset_name: str) -> Optional[Dict[str, Any]]:
        """Get information about a preset"""
        if preset_name not in self.presets:
            return None
            
        preset = self.presets[preset_name]
        return {
            'name': preset.name,
            'category': preset.category,
            'description': preset.description,
            'parameters': preset.parameters,
            'is_playing': preset_name in self.playing_sounds
        }
        
    def list_presets(self) -> List[Dict[str, str]]:
        """List all available presets"""
        return [
            {
                'name': preset.name,
                'category': preset.category,
                'description': preset.description
            }
            for preset in self.presets.values()
        ]

def main():
    """Main function for command-line usage"""
    parser = argparse.ArgumentParser(description='Audio Interface for JSON Sound Presets')
    parser.add_argument('--json-files', nargs='+', 
                       default=['electronic_track.json', 'guitar.json', 'group.json'],
                       help='JSON files to load')
    parser.add_argument('--engine', choices=['pygame', 'sounddevice', 'mock'], 
                       default=None, help='Audio engine to use')
    parser.add_argument('--demo', action='store_true', help='Run demo mode')
    parser.add_argument('--load-presets', action='store_true', help='Load and list presets')
    
    args = parser.parse_args()
    
    # Create audio interface
    engine = AudioEngine(args.engine) if args.engine else None
    audio_interface = AudioInterface(engine)
    
    # Load JSON datasets
    print(f"Loading JSON datasets: {args.json_files}")
    loaded_count = audio_interface.load_json_datasets(args.json_files)
    print(f"Loaded {loaded_count} presets total")
    
    if args.load_presets:
        presets = audio_interface.list_presets()
        print("\nAvailable presets:")
        for preset in presets:
            print(f"  {preset['category']}: {preset['name']} - {preset['description']}")
        return
        
    if args.demo:
        print("\nRunning demo...")
        presets = audio_interface.list_presets()
        if presets:
            # Play first few presets
            for i, preset in enumerate(presets[:3]):
                print(f"Playing: {preset['name']}")
                audio_interface.play_preset(preset['name'], duration=1.0)
                time.sleep(1.5)
        else:
            print("No presets available for demo")
            
    print(f"\nAudio interface ready with {loaded_count} presets")
    print("Use the web interface for interactive playback")

if __name__ == '__main__':
    main()