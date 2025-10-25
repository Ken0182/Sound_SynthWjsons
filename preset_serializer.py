"""
Preset serializer: converts NormalizedPreset objects to schema-compliant dicts.
"""
from typing import Any, Dict


def to_schema(normalized_preset: Any) -> Dict[str, Any]:
    env = normalized_preset.envelope
    filt = normalized_preset.filter
    osc = normalized_preset.oscillator
    fx = normalized_preset.fx

    return {
        "name": getattr(normalized_preset, "name", "Unnamed"),
        "role": getattr(normalized_preset, "role", None).value if getattr(normalized_preset, "role", None) else "UNKNOWN",
        "envelope": {
            "attack": float(getattr(env, "attack", 0.01)),
            "decay": float(getattr(env, "decay", 0.1)),
            "sustain": float(getattr(env, "sustain", 0.7)),
            "release": float(getattr(env, "release", 0.5)),
        },
        "filter": {
            "cutoff": float(getattr(filt, "cutoff", 1000.0)),
            "resonance": float(getattr(filt, "resonance", 0.3)),
            "envelope_amount": float(getattr(filt, "envelope_amount", 0.0)),
        },
        "oscillator": {
            "detune": float(getattr(osc, "detune", 1.0)),
            "mix_ratios": list(getattr(osc, "mix_ratios", [1.0])),
        },
        "fx": [
            {"type": getattr(e, "type", ""), "mix": float(getattr(e, "mix", 0.0))}
            for e in (fx or [])
        ],
        "parameters": getattr(normalized_preset, "sound_characteristics", {}),
        "metadata": getattr(normalized_preset, "metadata", {}),
    }
