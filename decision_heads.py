"""
Decision Heads for Ranges & Routings
Implements Step 6: Decision heads for ranges & routings
"""

from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass, field
import numpy as np
import random
from core_models import Role, NormalizedPreset

@dataclass
class Context:
    """Context for decision making"""
    query_vector: np.ndarray
    role: Role
    tempo: float
    key: str
    entry_vector: Optional[np.ndarray] = None
    preset_metadata: Optional[Dict] = None

@dataclass
class ValueDecision:
    """Decision for a parameter value"""
    parameter_name: str
    mu: float  # Mean value in [0,1]
    sigma: float  # Standard deviation for jitter
    min_val: float
    max_val: float
    final_value: float

@dataclass
class RoutingDecision:
    """Decision for modulation routing"""
    source: str  # mod_lfo, mod_env, macro_motion
    target: str  # parameter name
    amount: float  # modulation amount [0,1]
    enabled: bool

@dataclass
class Decisions:
    """Complete set of decisions"""
    value_decisions: List[ValueDecision] = field(default_factory=list)
    routing_decisions: List[RoutingDecision] = field(default_factory=list)
    jitter_seed: int = 0

class DecisionHead:
    """Base class for decision heads"""
    
    def __init__(self, name: str):
        self.name = name
    
    def infer(self, context: Context) -> List[ValueDecision]:
        """Infer parameter decisions from context"""
        raise NotImplementedError

class ValueHead(DecisionHead):
    """Head for inferring parameter values"""
    
    def __init__(self, parameter_name: str, min_val: float, max_val: float, 
                 role_weights: Optional[Dict[Role, float]] = None):
        super().__init__(f"value_{parameter_name}")
        self.parameter_name = parameter_name
        self.min_val = min_val
        self.max_val = max_val
        self.role_weights = role_weights or {}
    
    def infer(self, context: Context) -> List[ValueDecision]:
        """Infer value decision for this parameter"""
        # Base value from query vector (use first few dimensions)
        base_mu = self._extract_base_value(context.query_vector)
        
        # Apply role-specific weighting
        role_weight = self.role_weights.get(context.role, 1.0)
        mu = base_mu * role_weight
        
        # Clamp to [0,1]
        mu = np.clip(mu, 0.0, 1.0)
        
        # Calculate jitter amount based on role
        sigma = self._get_jitter_amount(context.role)
        
        # Apply jitter
        jitter_seed = hash(context.query_vector.tobytes()) % 2**32
        np.random.seed(jitter_seed)
        jitter = np.random.normal(0, sigma)
        final_mu = np.clip(mu + jitter, 0.0, 1.0)
        
        # Map to actual parameter range
        final_value = self.min_val + final_mu * (self.max_val - self.min_val)
        
        return [ValueDecision(
            parameter_name=self.parameter_name,
            mu=mu,
            sigma=sigma,
            min_val=self.min_val,
            max_val=self.max_val,
            final_value=final_value
        )]
    
    def _extract_base_value(self, query_vector: np.ndarray) -> float:
        """Extract base value from query vector"""
        # Use weighted combination of first few dimensions
        weights = np.array([0.4, 0.3, 0.2, 0.1])
        if len(query_vector) >= len(weights):
            return np.dot(query_vector[:len(weights)], weights)
        else:
            return np.mean(query_vector)
    
    def _get_jitter_amount(self, role: Role) -> float:
        """Get jitter amount based on role"""
        jitter_by_role = {
            Role.PAD: 0.05,      # Low jitter for stable pads
            Role.BASS: 0.03,     # Very low jitter for bass
            Role.LEAD: 0.08,     # Medium jitter for leads
            Role.FX: 0.15,       # High jitter for effects
            Role.TEXTURE: 0.10,  # Medium-high jitter for textures
            Role.ARP: 0.12,      # High jitter for arpeggiators
            Role.DRONE: 0.02,    # Very low jitter for drones
            Role.RHYTHM: 0.08,   # Medium jitter for rhythm
            Role.BELL: 0.06,     # Low-medium jitter for bells
            Role.CHORD: 0.05,    # Low jitter for chords
            Role.PLUCK: 0.07     # Medium jitter for plucks
        }
        return jitter_by_role.get(role, 0.05)

class RoutingHead(DecisionHead):
    """Head for inferring modulation routing"""
    
    def __init__(self, source: str, available_targets: List[str]):
        super().__init__(f"routing_{source}")
        self.source = source
        self.available_targets = available_targets
    
    def infer(self, context: Context) -> List[RoutingDecision]:
        """Infer routing decisions for this source"""
        decisions = []
        
        # Determine which targets to connect based on context
        for target in self.available_targets:
            # Calculate connection probability based on query and role
            connection_prob = self._calculate_connection_probability(context, target)
            
            # Add some randomness
            jitter_seed = hash((context.query_vector.tobytes(), target)) % 2**32
            np.random.seed(jitter_seed)
            random_factor = np.random.uniform(0.8, 1.2)
            final_prob = np.clip(connection_prob * random_factor, 0.0, 1.0)
            
            # Decide if connection should be enabled
            enabled = final_prob > 0.5
            
            if enabled:
                # Calculate modulation amount
                amount = self._calculate_modulation_amount(context, target, final_prob)
                
                decisions.append(RoutingDecision(
                    source=self.source,
                    target=target,
                    amount=amount,
                    enabled=enabled
                ))
        
        return decisions
    
    def _calculate_connection_probability(self, context: Context, target: str) -> float:
        """Calculate probability of connecting source to target"""
        base_prob = 0.3  # Base connection probability
        
        # Role-based adjustments
        role_adjustments = {
            Role.PAD: {
                'filter_cutoff': 0.8,
                'filter_resonance': 0.6,
                'oscillator_detune': 0.4,
                'reverb_amount': 0.7
            },
            Role.BASS: {
                'filter_cutoff': 0.9,
                'filter_resonance': 0.7,
                'oscillator_detune': 0.2,
                'reverb_amount': 0.3
            },
            Role.LEAD: {
                'filter_cutoff': 0.8,
                'filter_resonance': 0.6,
                'oscillator_detune': 0.7,
                'reverb_amount': 0.5
            },
            Role.FX: {
                'filter_cutoff': 0.9,
                'filter_resonance': 0.8,
                'oscillator_detune': 0.8,
                'reverb_amount': 0.6
            }
        }
        
        role_adj = role_adjustments.get(context.role, {})
        target_adj = role_adj.get(target, 1.0)
        
        # Query-based adjustments (use vector magnitude as activity indicator)
        query_activity = np.linalg.norm(context.query_vector)
        activity_factor = 0.5 + 0.5 * query_activity  # Scale to [0.5, 1.0]
        
        final_prob = base_prob * target_adj * activity_factor
        return np.clip(final_prob, 0.0, 1.0)
    
    def _calculate_modulation_amount(self, context: Context, target: str, probability: float) -> float:
        """Calculate modulation amount for a connection"""
        base_amount = probability  # Use probability as base amount
        
        # Role-based scaling
        role_scaling = {
            Role.PAD: 0.3,      # Subtle modulation for pads
            Role.BASS: 0.2,     # Very subtle for bass
            Role.LEAD: 0.5,     # Moderate for leads
            Role.FX: 0.8,       # Strong for effects
            Role.TEXTURE: 0.6,  # Medium-strong for textures
            Role.ARP: 0.7,      # Strong for arpeggiators
            Role.DRONE: 0.1,    # Very subtle for drones
            Role.RHYTHM: 0.6,   # Medium-strong for rhythm
            Role.BELL: 0.4,     # Moderate for bells
            Role.CHORD: 0.3,    # Subtle for chords
            Role.PLUCK: 0.5     # Moderate for plucks
        }
        
        scaling = role_scaling.get(context.role, 0.5)
        return np.clip(base_amount * scaling, 0.0, 1.0)

class DecisionEngine:
    """Main decision engine that coordinates all heads"""
    
    def __init__(self):
        self.value_heads: List[ValueHead] = []
        self.routing_heads: List[RoutingHead] = []
        self._setup_heads()
    
    def _setup_heads(self):
        """Set up all decision heads"""
        # Value heads for common parameters
        self.value_heads = [
            ValueHead("filter_cutoff", 20, 20000, {
                Role.PAD: 0.6,
                Role.BASS: 0.3,
                Role.LEAD: 0.8,
                Role.FX: 1.0
            }),
            ValueHead("filter_resonance", 0.0, 0.9, {
                Role.PAD: 0.3,
                Role.BASS: 0.6,
                Role.LEAD: 0.5,
                Role.FX: 0.8
            }),
            ValueHead("oscillator_detune", 0.0, 0.1, {
                Role.PAD: 0.4,
                Role.BASS: 0.1,
                Role.LEAD: 0.7,
                Role.FX: 0.9
            }),
            ValueHead("reverb_amount", 0.0, 1.0, {
                Role.PAD: 0.8,
                Role.BASS: 0.2,
                Role.LEAD: 0.5,
                Role.FX: 0.7
            }),
            ValueHead("delay_amount", 0.0, 1.0, {
                Role.PAD: 0.4,
                Role.BASS: 0.1,
                Role.LEAD: 0.6,
                Role.FX: 0.8
            }),
            ValueHead("distortion_amount", 0.0, 1.0, {
                Role.PAD: 0.1,
                Role.BASS: 0.3,
                Role.LEAD: 0.5,
                Role.FX: 0.8
            }),
            ValueHead("stereo_width", 0.0, 1.0, {
                Role.PAD: 1.0,
                Role.BASS: 0.0,
                Role.LEAD: 0.5,
                Role.FX: 1.0
            }),
            ValueHead("lfo_rate", 0.1, 10.0, {
                Role.PAD: 0.5,
                Role.BASS: 0.3,
                Role.LEAD: 0.7,
                Role.FX: 1.0
            })
        ]
        
        # Routing heads
        available_targets = [
            "filter_cutoff", "filter_resonance", "oscillator_detune",
            "reverb_amount", "delay_amount", "distortion_amount",
            "stereo_width", "lfo_rate"
        ]
        
        self.routing_heads = [
            RoutingHead("mod_lfo", available_targets),
            RoutingHead("mod_env", available_targets),
            RoutingHead("macro_motion", available_targets)
        ]
    
    def infer_decisions(self, context: Context) -> Decisions:
        """Infer all decisions from context"""
        # Set random seed for reproducible jitter
        jitter_seed = hash(context.query_vector.tobytes()) % 2**32
        np.random.seed(jitter_seed)
        
        # Collect value decisions
        value_decisions = []
        for head in self.value_heads:
            decisions = head.infer(context)
            value_decisions.extend(decisions)
        
        # Collect routing decisions
        routing_decisions = []
        for head in self.routing_heads:
            decisions = head.infer(context)
            routing_decisions.extend(decisions)
        
        return Decisions(
            value_decisions=value_decisions,
            routing_decisions=routing_decisions,
            jitter_seed=jitter_seed
        )
    
    def apply_decisions(self, preset: NormalizedPreset, decisions: Decisions) -> Dict[str, Any]:
        """Apply decisions to a preset"""
        applied_changes = {}
        
        # Apply value decisions
        for decision in decisions.value_decisions:
            if decision.parameter_name == "filter_cutoff":
                preset.filter.cutoff = decision.final_value
                applied_changes["filter_cutoff"] = decision.final_value
            elif decision.parameter_name == "filter_resonance":
                preset.filter.resonance = decision.final_value
                applied_changes["filter_resonance"] = decision.final_value
            elif decision.parameter_name == "oscillator_detune":
                preset.oscillator.detune = decision.final_value
                applied_changes["oscillator_detune"] = decision.final_value
            # Add more parameter mappings as needed
        
        # Apply routing decisions (store for later use in graph building)
        routing_info = {}
        for decision in decisions.routing_decisions:
            if decision.enabled:
                if decision.source not in routing_info:
                    routing_info[decision.source] = []
                routing_info[decision.source].append({
                    "target": decision.target,
                    "amount": decision.amount
                })
        
        applied_changes["routing"] = routing_info
        
        return applied_changes
    
    def validate_decisions(self, decisions: Decisions) -> List[str]:
        """Validate that all decisions are within bounds"""
        issues = []
        
        # Validate value decisions
        for decision in decisions.value_decisions:
            if decision.final_value < decision.min_val or decision.final_value > decision.max_val:
                issues.append(f"Parameter {decision.parameter_name} value {decision.final_value} "
                            f"outside range [{decision.min_val}, {decision.max_val}]")
        
        # Validate routing decisions
        for decision in decisions.routing_decisions:
            if decision.amount < 0.0 or decision.amount > 1.0:
                issues.append(f"Routing {decision.source}->{decision.target} amount {decision.amount} "
                            f"outside range [0.0, 1.0]")
        
        return issues

if __name__ == "__main__":
    from core_models import JsonPresetParser, Role
    from normalizer import UnitNormalizer
    
    # Test the decision heads system
    parser = JsonPresetParser()
    presets = parser.parse_from_file('group.json')
    
    if presets:
        # Normalize a preset
        normalizer = UnitNormalizer()
        normalized_preset = normalizer.normalize(presets[0])
        
        # Create decision engine
        decision_engine = DecisionEngine()
        
        # Create test context
        query_vector = np.random.randn(128)  # Random query vector
        context = Context(
            query_vector=query_vector,
            role=Role.PAD,
            tempo=120.0,
            key="C"
        )
        
        # Infer decisions
        decisions = decision_engine.infer_decisions(context)
        
        print(f"Inferred {len(decisions.value_decisions)} value decisions and {len(decisions.routing_decisions)} routing decisions")
        
        # Show some decisions
        print("\nValue decisions:")
        for decision in decisions.value_decisions[:5]:  # Show first 5
            print(f"  {decision.parameter_name}: {decision.final_value:.3f} (μ={decision.mu:.3f}, σ={decision.sigma:.3f})")
        
        print("\nRouting decisions:")
        for decision in decisions.routing_decisions[:5]:  # Show first 5
            if decision.enabled:
                print(f"  {decision.source} -> {decision.target}: {decision.amount:.3f}")
        
        # Validate decisions
        issues = decision_engine.validate_decisions(decisions)
        if issues:
            print(f"\nValidation issues: {len(issues)}")
            for issue in issues[:3]:  # Show first 3
                print(f"  - {issue}")
        else:
            print("\nAll decisions validated successfully")