"""
Deterministic Control Graph Builder
Implements Step 2: Map JSON schema → Deterministic Control Graph
"""

from typing import Dict, List, Tuple, Optional, Set
from dataclasses import dataclass
from core_models import JsonPreset, Graph, GraphNode, SynthesisType, OscillatorType, FilterType, EffectConfig
import math

class GraphBuilder:
    """Builds deterministic control graphs from JSON presets"""
    
    def __init__(self):
        self.node_counter = 0
        self.virtual_busses = {
            'mod_lfo': [],
            'mod_env': [],
            'macro_motion': []
        }
    
    def build_graph(self, preset: JsonPreset) -> Graph:
        """Build a playable DSP graph from a JSON preset"""
        graph = Graph()
        
        # Signal order: source → shaper → filter → spatial → limiter
        self._add_oscillator_nodes(graph, preset)
        self._add_envelope_nodes(graph, preset)
        self._add_filter_nodes(graph, preset)
        self._add_effect_nodes(graph, preset)
        self._add_spatial_nodes(graph, preset)
        self._add_limiter_nodes(graph, preset)
        
        # Connect the signal chain
        self._connect_signal_chain(graph)
        
        # Add modulation routing
        self._add_modulation_routing(graph, preset)
        
        # Validate the graph
        graph.validation_passed, graph.validation_errors = self._validate_graph(graph)
        
        return graph
    
    def _add_oscillator_nodes(self, graph: Graph, preset: JsonPreset):
        """Add oscillator source nodes"""
        for i, osc_type in enumerate(preset.oscillator.types):
            node_id = f"osc_{i}_{osc_type.value}"
            
            # Map oscillator type to DSP parameters
            params = self._get_oscillator_params(osc_type, preset.oscillator, i)
            
            node = GraphNode(
                id=node_id,
                node_type="oscillator",
                parameters=params,
                outputs=["audio_out"]
            )
            graph.nodes.append(node)
    
    def _add_envelope_nodes(self, graph: Graph, preset: JsonPreset):
        """Add envelope generator nodes"""
        node_id = "env_main"
        
        # Convert envelope times to seconds
        attack = self._parse_time_to_seconds(preset.envelope.attack)
        decay = self._parse_time_to_seconds(preset.envelope.decay)
        sustain = preset.envelope.sustain if isinstance(preset.envelope.sustain, float) else 0.7
        release = self._parse_time_to_seconds(preset.envelope.release)
        
        params = {
            "attack": attack,
            "decay": decay,
            "sustain": sustain,
            "release": release,
            "curve": preset.envelope.curve.value,
            "type": preset.envelope.type.value
        }
        
        # Add hold and delay if present
        if hasattr(preset.envelope, 'hold') and preset.envelope.hold:
            params["hold"] = self._parse_time_to_seconds(preset.envelope.hold)
        if hasattr(preset.envelope, 'delay') and preset.envelope.delay:
            params["delay"] = self._parse_time_to_seconds(preset.envelope.delay)
        
        node = GraphNode(
            id=node_id,
            node_type="envelope",
            parameters=params,
            inputs=["gate_in"],
            outputs=["env_out"]
        )
        graph.nodes.append(node)
        
        # Add to modulation bus
        self.virtual_busses['mod_env'].append(node_id)
    
    def _add_filter_nodes(self, graph: Graph, preset: JsonPreset):
        """Add filter nodes"""
        node_id = "filter_main"
        
        # Convert cutoff to Hz
        cutoff = self._parse_frequency_to_hz(preset.filter.cutoff)
        resonance = preset.filter.resonance if isinstance(preset.filter.resonance, float) else 0.5
        
        params = {
            "type": preset.filter.type.value,
            "cutoff": cutoff,
            "resonance": resonance,
            "envelope_amount": preset.filter.envelope_amount if isinstance(preset.filter.envelope_amount, float) else 0.0,
            "slope": preset.filter.slope
        }
        
        node = GraphNode(
            id=node_id,
            node_type="filter",
            parameters=params,
            inputs=["audio_in", "mod_in"],
            outputs=["audio_out"]
        )
        graph.nodes.append(node)
    
    def _add_effect_nodes(self, graph: Graph, preset: JsonPreset):
        """Add effect processing nodes"""
        for i, effect in enumerate(preset.fx):
            node_id = f"fx_{i}_{effect.type}"
            
            params = self._get_effect_params(effect)
            
            node = GraphNode(
                id=node_id,
                node_type="effect",
                parameters=params,
                inputs=["audio_in"],
                outputs=["audio_out"]
            )
            graph.nodes.append(node)
    
    def _add_spatial_nodes(self, graph: Graph, preset: JsonPreset):
        """Add spatial processing nodes (stereo, reverb, etc.)"""
        # Add stereo width control
        node_id = "stereo_width"
        params = {
            "width": 1.0,  # Default to full stereo
            "pan": 0.0
        }
        
        node = GraphNode(
            id=node_id,
            node_type="stereo",
            parameters=params,
            inputs=["audio_in"],
            outputs=["left_out", "right_out"]
        )
        graph.nodes.append(node)
    
    def _add_limiter_nodes(self, graph: Graph, preset: JsonPreset):
        """Add final limiting and safety nodes"""
        # True-peak limiter
        node_id = "limiter_tp"
        params = {
            "threshold": -1.0,  # -1 dBTP
            "release": 0.1,
            "lookahead": 0.005
        }
        
        node = GraphNode(
            id=node_id,
            node_type="limiter",
            parameters=params,
            inputs=["audio_in"],
            outputs=["audio_out"]
        )
        graph.nodes.append(node)
        
        # Soft clipper for character
        node_id = "clipper_soft"
        params = {
            "threshold": -3.0,
            "ratio": 0.1
        }
        
        node = GraphNode(
            id=node_id,
            node_type="clipper",
            parameters=params,
            inputs=["audio_in"],
            outputs=["audio_out"]
        )
        graph.nodes.append(node)
    
    def _connect_signal_chain(self, graph: Graph):
        """Connect the main signal chain"""
        # Find nodes in order
        osc_nodes = [n for n in graph.nodes if n.node_type == "oscillator"]
        env_node = next((n for n in graph.nodes if n.node_type == "envelope"), None)
        filter_node = next((n for n in graph.nodes if n.node_type == "filter"), None)
        fx_nodes = [n for n in graph.nodes if n.node_type == "effect"]
        stereo_node = next((n for n in graph.nodes if n.node_type == "stereo"), None)
        limiter_node = next((n for n in graph.nodes if n.node_type == "limiter"), None)
        clipper_node = next((n for n in graph.nodes if n.node_type == "clipper"), None)
        
        # Connect oscillators to filter
        if osc_nodes and filter_node:
            for osc in osc_nodes:
                graph.connections.append((osc.id, "audio_out", filter_node.id, "audio_in"))
        
        # Connect envelope to filter modulation
        if env_node and filter_node:
            graph.connections.append((env_node.id, "env_out", filter_node.id, "mod_in"))
        
        # Connect filter to effects
        current_node = filter_node
        for fx in fx_nodes:
            if current_node:
                graph.connections.append((current_node.id, "audio_out", fx.id, "audio_in"))
            current_node = fx
        
        # Connect to stereo processing
        if current_node and stereo_node:
            graph.connections.append((current_node.id, "audio_out", stereo_node.id, "audio_in"))
        
        # Connect to limiter chain
        if stereo_node and clipper_node:
            graph.connections.append((stereo_node.id, "left_out", clipper_node.id, "audio_in"))
            graph.connections.append((stereo_node.id, "right_out", clipper_node.id, "audio_in"))
        
        if clipper_node and limiter_node:
            graph.connections.append((clipper_node.id, "audio_out", limiter_node.id, "audio_in"))
    
    def _add_modulation_routing(self, graph: Graph, preset: JsonPreset):
        """Add modulation routing based on preset configuration"""
        # Add LFO for modulation
        lfo_node_id = "lfo_main"
        params = {
            "frequency": 0.5,  # Hz
            "waveform": "sine",
            "depth": 0.1
        }
        
        lfo_node = GraphNode(
            id=lfo_node_id,
            node_type="lfo",
            parameters=params,
            outputs=["lfo_out"]
        )
        graph.nodes.append(lfo_node)
        
        # Add to modulation bus
        self.virtual_busses['mod_lfo'].append(lfo_node_id)
        
        # Connect LFO to filter cutoff modulation
        filter_node = next((n for n in graph.nodes if n.node_type == "filter"), None)
        if filter_node:
            graph.connections.append((lfo_node_id, "lfo_out", filter_node.id, "mod_in"))
    
    def _get_oscillator_params(self, osc_type: OscillatorType, osc_config, index: int) -> Dict:
        """Get DSP parameters for oscillator type"""
        params = {
            "type": osc_type.value,
            "mix": osc_config.mix_ratios[index] if index < len(osc_config.mix_ratios) else 1.0,
            "detune": osc_config.detune
        }
        
        if osc_type == OscillatorType.FM:
            params["modulation_index"] = osc_config.modulation_index or 1.0
            params["carrier_ratio"] = osc_config.carrier_ratio or 1.0
        elif osc_type == OscillatorType.WAVETABLE:
            params["table_index"] = osc_config.table_index or 0
            params["morph_rate"] = self._parse_frequency_to_hz(osc_config.morph_rate or "0.1Hz")
        elif osc_type == OscillatorType.GRANULAR:
            params["grain_density"] = self._parse_grain_density(osc_config.grain_density or "80/s")
            params["grain_size"] = self._parse_time_to_seconds(osc_config.grain_size or "30ms")
        elif osc_type == OscillatorType.ADDITIVE:
            params["harmonics"] = osc_config.harmonics or [1.0]
        
        return params
    
    def _get_effect_params(self, effect: EffectConfig) -> Dict:
        """Get DSP parameters for effect"""
        params = {
            "type": effect.type,
            "mix": effect.mix if isinstance(effect.mix, float) else 0.5
        }
        
        # Add effect-specific parameters
        if effect.feedback is not None:
            params["feedback"] = effect.feedback if isinstance(effect.feedback, float) else 0.3
        if effect.time is not None:
            params["time"] = self._parse_time_to_seconds(effect.time)
        if effect.gain is not None:
            params["gain"] = effect.gain if isinstance(effect.gain, float) else 0.5
        if effect.decay is not None:
            params["decay"] = self._parse_time_to_seconds(effect.decay)
        if effect.wet is not None:
            params["wet"] = effect.wet if isinstance(effect.wet, float) else 0.5
        if effect.rate is not None:
            params["rate"] = self._parse_frequency_to_hz(effect.rate)
        if effect.depth is not None:
            params["depth"] = effect.depth if isinstance(effect.depth, float) else 0.5
        
        return params
    
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
            return (time_value[0] + time_value[1]) / 2.0 / 1000.0  # Average of range
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
            return (freq_value[0] + freq_value[1]) / 2.0  # Average of range
        return 1000.0
    
    def _parse_grain_density(self, density_str: str) -> float:
        """Parse grain density string to grains per second"""
        if density_str.endswith('/s'):
            return float(density_str[:-2])
        return 80.0
    
    def _validate_graph(self, graph: Graph) -> Tuple[bool, List[str]]:
        """Validate graph topology and parameter ranges"""
        errors = []
        
        # Check for feedback loops
        if self._has_feedback_loops(graph):
            errors.append("Feedback loop detected in graph")
        
        # Check parameter ranges
        for node in graph.nodes:
            if node.node_type == "filter":
                cutoff = node.parameters.get("cutoff", 1000)
                if cutoff < 20 or cutoff > 20000:
                    errors.append(f"Filter cutoff {cutoff}Hz out of range [20, 20000]")
                
                resonance = node.parameters.get("resonance", 0)
                if resonance > 0.9:
                    errors.append(f"Filter resonance {resonance} exceeds maximum 0.9")
            
            elif node.node_type == "effect":
                if node.parameters.get("type") == "delay":
                    feedback = node.parameters.get("feedback", 0)
                    if feedback > 0.85:
                        errors.append(f"Delay feedback {feedback} exceeds maximum 0.85")
        
        # Check for disconnected nodes
        connected_nodes = set()
        for from_node, from_port, to_node, to_port in graph.connections:
            connected_nodes.add(from_node)
            connected_nodes.add(to_node)
        
        for node in graph.nodes:
            if node.id not in connected_nodes and node.node_type not in ["lfo", "envelope"]:
                errors.append(f"Node {node.id} is not connected to signal chain")
        
        return len(errors) == 0, errors
    
    def _has_feedback_loops(self, graph: Graph) -> bool:
        """Check for feedback loops in the graph"""
        # Simple cycle detection using DFS
        visited = set()
        rec_stack = set()
        
        def has_cycle(node_id):
            visited.add(node_id)
            rec_stack.add(node_id)
            
            # Find outgoing connections
            for from_node, from_port, to_node, to_port in graph.connections:
                if from_node == node_id:
                    if to_node in rec_stack:
                        return True
                    if to_node not in visited and has_cycle(to_node):
                        return True
            
            rec_stack.remove(node_id)
            return False
        
        for node in graph.nodes:
            if node.id not in visited:
                if has_cycle(node.id):
                    return True
        
        return False

if __name__ == "__main__":
    from core_models import JsonPresetParser
    
    # Test the graph builder
    parser = JsonPresetParser()
    presets = parser.parse_from_file('group.json')
    
    if presets:
        builder = GraphBuilder()
        graph = builder.build_graph(presets[0])
        
        print(f"Built graph with {len(graph.nodes)} nodes and {len(graph.connections)} connections")
        print(f"Validation passed: {graph.validation_passed}")
        if graph.validation_errors:
            print("Validation errors:")
            for error in graph.validation_errors:
                print(f"  - {error}")
        
        print("\nNodes:")
        for node in graph.nodes:
            print(f"  {node.id}: {node.node_type}")
        
        print("\nConnections:")
        for conn in graph.connections:
            print(f"  {conn[0]}.{conn[1]} -> {conn[2]}.{conn[3]}")