"""
JSON-to-Audio Preset Generator - Main Application
Implements the complete 20-step system for converting JSON presets to audio
"""

import argparse
import json
import sys
from typing import List, Dict, Any, Optional
import numpy as np

from core_models import JsonPresetParser, Role
from normalizer import UnitNormalizer
from graph_builder import GraphBuilder
from semantic_embeddings import SemanticSearchEngine
from roles_policies import RolePolicyManager
from decision_heads import DecisionEngine, Context
from cpp_engine import create_hybrid_interface, CPP_ENGINE_AVAILABLE

class AudioPresetGenerator:
    """Main class that orchestrates the entire JSON-to-Audio pipeline"""
    
    def __init__(self, use_cpp_engine: bool = True):
        self.parser = JsonPresetParser()
        self.normalizer = UnitNormalizer()
        self.graph_builder = GraphBuilder()
        self.search_engine = SemanticSearchEngine()
        self.policy_manager = RolePolicyManager()
        self.decision_engine = DecisionEngine()
        self.presets: List[Any] = []
        self.normalized_presets: List[Any] = []
        
        # Initialize C++ engine if available
        self.use_cpp_engine = use_cpp_engine and CPP_ENGINE_AVAILABLE
        self.cpp_engine = None
        if self.use_cpp_engine:
            try:
                self.cpp_engine = create_hybrid_interface(use_cpp=True)
                print("C++ engine initialized successfully")
            except Exception as e:
                print(f"Failed to initialize C++ engine: {e}")
                self.use_cpp_engine = False
    
    def load_presets(self, file_paths: List[str]) -> None:
        """Load presets from JSON files"""
        print(f"Loading presets from {len(file_paths)} files...")
        
        all_presets = []
        for file_path in file_paths:
            try:
                presets = self.parser.parse_from_file(file_path)
                all_presets.extend(presets)
                print(f"  Loaded {len(presets)} presets from {file_path}")
            except Exception as e:
                print(f"  Error loading {file_path}: {e}")
        
        self.presets = all_presets
        print(f"Total presets loaded: {len(self.presets)}")
    
    def normalize_presets(self) -> None:
        """Normalize all presets to SI units and canonical ranges"""
        print("Normalizing presets...")
        
        self.normalized_presets = []
        for preset in self.presets:
            try:
                normalized = self.normalizer.normalize(preset)
                self.normalized_presets.append(normalized)
            except Exception as e:
                print(f"  Error normalizing preset {preset.name}: {e}")
        
        print(f"Normalized {len(self.normalized_presets)} presets")
    
    def build_search_index(self) -> None:
        """Build semantic search index"""
        print("Building semantic search index...")
        self.search_engine.build_index(self.normalized_presets)
        print("Search index built successfully")
    
    def search_presets(self, query: str, role: Optional[str] = None, 
                      tempo: Optional[float] = None, key: Optional[str] = None, 
                      top_k: int = 10) -> List[tuple]:
        """Search for presets matching query"""
        print(f"Searching for: '{query}'")
        if role:
            print(f"  Role: {role}")
        if tempo:
            print(f"  Tempo: {tempo} BPM")
        if key:
            print(f"  Key: {key}")
        
        results = self.search_engine.search(query, role, tempo, key, top_k)
        
        print(f"Found {len(results)} results:")
        for i, (preset_id, score) in enumerate(results):
            print(f"  {i+1}. {preset_id}: {score:.3f}")
        
        return results
    
    def generate_audio_preset(self, query: str, role: Optional[str] = None,
                             tempo: Optional[float] = None, key: Optional[str] = None) -> Dict[str, Any]:
        """Generate a complete audio preset from a text query"""
        print(f"\nGenerating audio preset for: '{query}'")
        
        # Step 1: Search for similar presets
        search_results = self.search_presets(query, role, tempo, key, top_k=5)
        
        if not search_results:
            print("No matching presets found")
            return {}
        
        # Step 2: Get the best matching preset
        best_preset_id, best_score = search_results[0]
        print(f"Using base preset: {best_preset_id} (score: {best_score:.3f})")
        
        # Find the normalized preset
        base_preset = None
        for preset in self.normalized_presets:
            if preset.name == best_preset_id:
                base_preset = preset
                break
        
        if not base_preset:
            print(f"Preset {best_preset_id} not found in normalized presets")
            return {}
        
        # Step 3: Determine role
        if role:
            try:
                role_enum = Role(role.lower())
            except ValueError:
                print(f"Invalid role: {role}, using preset's default role")
                role_enum = base_preset.role
        else:
            role_enum = base_preset.role
        
        print(f"Using role: {role_enum.value}")
        
        # Step 4: Apply role policy
        policy_adjustments = self.policy_manager.apply_policy(
            base_preset, role_enum, tempo or 120.0, key or "C"
        )
        print(f"Applied role policy adjustments: {len(policy_adjustments)} categories")
        
        # Step 5: Generate query embedding for decision making
        query_vector = self.search_engine.embedding_generator.generate_query_embedding(
            query, role, tempo, key
        ).vector
        
        # Step 6: Create context for decision making
        context = Context(
            query_vector=query_vector,
            role=role_enum,
            tempo=tempo or 120.0,
            key=key or "C",
            entry_vector=self.search_engine.embedding_generator.entry_embeddings.get(best_preset_id),
            preset_metadata=base_preset.metadata
        )
        
        # Step 7: Infer decisions
        decisions = self.decision_engine.infer_decisions(context)
        print(f"Generated {len(decisions.value_decisions)} value decisions and {len(decisions.routing_decisions)} routing decisions")
        
        # Step 8: Apply decisions to preset
        applied_changes = self.decision_engine.apply_decisions(base_preset, decisions)
        print(f"Applied {len(applied_changes)} parameter changes")
        
        # Step 9: Build DSP graph
        graph = self.graph_builder.build_graph(base_preset)
        print(f"Built DSP graph with {len(graph.nodes)} nodes and {len(graph.connections)} connections")
        
        if not graph.validation_passed:
            print(f"Graph validation failed: {len(graph.validation_errors)} errors")
            for error in graph.validation_errors[:3]:  # Show first 3 errors
                print(f"  - {error}")
        else:
            print("Graph validation passed")
        
        # Step 10: Generate audio using C++ engine if available
        audio_data = None
        if self.use_cpp_engine and self.cpp_engine:
            try:
                # Convert preset to audio using C++ engine
                preset_dict = {
                    "frequency": base_preset.oscillator.frequency,
                    "waveform": base_preset.oscillator.waveform,
                    "attack": base_preset.envelope.attack,
                    "decay": base_preset.envelope.decay,
                    "sustain": base_preset.envelope.sustain,
                    "release": base_preset.envelope.release,
                    "cutoff": base_preset.filter.cutoff,
                    "resonance": base_preset.filter.resonance
                }
                audio_data = self.cpp_engine.generate_from_preset(preset_dict, duration=2.0)
                print(f"Generated audio using C++ engine: {len(audio_data)} samples")
            except Exception as e:
                print(f"Error generating audio with C++ engine: {e}")
                audio_data = None
        
        # Step 11: Compile final preset
        final_preset = {
            "name": f"Generated_{query.replace(' ', '_')}",
            "base_preset": best_preset_id,
            "role": role_enum.value,
            "query": query,
            "tempo": tempo or 120.0,
            "key": key or "C",
            "similarity_score": best_score,
            "policy_adjustments": policy_adjustments,
            "applied_changes": applied_changes,
            "graph": {
                "nodes": len(graph.nodes),
                "connections": len(graph.connections),
                "validation_passed": graph.validation_passed,
                "validation_errors": graph.validation_errors
            },
            "parameters": {
                "filter_cutoff": base_preset.filter.cutoff,
                "filter_resonance": base_preset.filter.resonance,
                "oscillator_detune": base_preset.oscillator.detune,
                "envelope_attack": base_preset.envelope.attack,
                "envelope_release": base_preset.envelope.release
            },
            "audio_data": audio_data.tolist() if audio_data is not None else None,
            "cpp_engine_used": self.use_cpp_engine
        }
        
        return final_preset
    
    def explain_preset(self, preset: Dict[str, Any]) -> str:
        """Generate human-readable explanation of a preset"""
        explanation = f"Generated preset '{preset['name']}' based on query '{preset['query']}':\n\n"
        
        explanation += f"• Base preset: {preset['base_preset']} (similarity: {preset['similarity_score']:.3f})\n"
        explanation += f"• Role: {preset['role']}\n"
        explanation += f"• Tempo: {preset['tempo']} BPM, Key: {preset['key']}\n\n"
        
        explanation += "Key parameters:\n"
        for param, value in preset['parameters'].items():
            if isinstance(value, float):
                explanation += f"• {param}: {value:.3f}\n"
            else:
                explanation += f"• {param}: {value}\n"
        
        explanation += f"\nDSP Graph: {preset['graph']['nodes']} nodes, {preset['graph']['connections']} connections\n"
        if preset['graph']['validation_passed']:
            explanation += "✓ Graph validation passed\n"
        else:
            explanation += f"✗ Graph validation failed ({len(preset['graph']['validation_errors'])} errors)\n"
        
        return explanation
    
    def generate_audio(self, query: str, role: Optional[str] = None,
                      tempo: Optional[float] = None, key: Optional[str] = None,
                      duration: float = 2.0) -> np.ndarray:
        """Generate audio directly from query using the best available engine"""
        if self.use_cpp_engine and self.cpp_engine:
            # Use C++ engine for direct audio generation
            try:
                audio = self.cpp_engine.generate_audio(
                    prompt=query,
                    role=role or "UNKNOWN",
                    tempo=tempo or 120.0,
                    key=key or "C"
                )
                return audio
            except Exception as e:
                print(f"Error with C++ engine: {e}")
                # Fallback to Python generation
                pass
        
        # Fallback to Python-only generation
        print("Using Python-only audio generation")
        
        # Generate a simple tone based on query
        sample_rate = 44100
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        
        # Simple frequency mapping based on role
        if role == "BASS":
            frequency = 80.0
        elif role == "LEAD":
            frequency = 440.0
        elif role == "PAD":
            frequency = 220.0
        else:
            frequency = 440.0
        
        # Generate audio
        audio = np.sin(2 * np.pi * frequency * t)
        
        # Apply simple envelope
        attack_samples = int(0.1 * sample_rate)
        release_samples = int(0.5 * sample_rate)
        
        if len(audio) > attack_samples:
            audio[:attack_samples] *= np.linspace(0, 1, attack_samples)
        if len(audio) > release_samples:
            audio[-release_samples:] *= np.linspace(1, 0, release_samples)
        
        return audio.astype(np.float32)

def main():
    """Main application entry point"""
    parser = argparse.ArgumentParser(description="JSON-to-Audio Preset Generator")
    parser.add_argument("--files", nargs="+", default=["group.json", "electronic_track.json", "guitar.json"],
                       help="JSON files to load")
    parser.add_argument("--query", type=str, required=True,
                       help="Text query for preset generation")
    parser.add_argument("--role", type=str, choices=["pad", "bass", "lead", "fx", "texture", "arp", "drone", "rhythm", "bell", "chord", "pluck"],
                       help="Musical role")
    parser.add_argument("--tempo", type=float, help="Tempo in BPM")
    parser.add_argument("--key", type=str, help="Musical key")
    parser.add_argument("--search-only", action="store_true",
                       help="Only perform search, don't generate preset")
    parser.add_argument("--explain", action="store_true",
                       help="Generate explanation of the preset")
    parser.add_argument("--generate-audio", action="store_true",
                       help="Generate audio file from preset")
    parser.add_argument("--output", type=str, default="output.wav",
                       help="Output audio file path")
    parser.add_argument("--duration", type=float, default=2.0,
                       help="Audio duration in seconds")
    parser.add_argument("--use-cpp", action="store_true", default=True,
                       help="Use C++ engine for audio generation")
    
    args = parser.parse_args()
    
    # Create generator
    generator = AudioPresetGenerator(use_cpp_engine=args.use_cpp)
    
    try:
        # Load and process presets
        generator.load_presets(args.files)
        generator.normalize_presets()
        generator.build_search_index()
        
        if args.search_only:
            # Just perform search
            results = generator.search_presets(args.query, args.role, args.tempo, args.key)
        else:
            # Generate complete preset
            preset = generator.generate_audio_preset(args.query, args.role, args.tempo, args.key)
            
            if preset:
                print("\n" + "="*60)
                print("GENERATED PRESET")
                print("="*60)
                
                if args.explain:
                    print(generator.explain_preset(preset))
                else:
                    print(json.dumps(preset, indent=2, default=str))
                
                # Generate audio if requested
                if args.generate_audio:
                    print("\n" + "="*60)
                    print("GENERATING AUDIO")
                    print("="*60)
                    
                    audio = generator.generate_audio(
                        args.query, args.role, args.tempo, args.key, args.duration
                    )
                    
                    if audio is not None and len(audio) > 0:
                        # Save audio to file
                        try:
                            import soundfile as sf
                            sf.write(args.output, audio, 44100)
                            print(f"Audio saved to: {args.output}")
                            print(f"Duration: {len(audio) / 44100:.2f} seconds")
                            print(f"Samples: {len(audio)}")
                        except ImportError:
                            print("Warning: soundfile not available, cannot save audio")
                            print(f"Generated {len(audio)} audio samples")
                        except Exception as e:
                            print(f"Error saving audio: {e}")
                    else:
                        print("Failed to generate audio")
            else:
                print("Failed to generate preset")
                sys.exit(1)
    
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()