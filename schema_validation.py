"""
Lightweight schema validation hooks for preset and request dicts.
Avoids external deps; checks keys and simple ranges per YAML spec.
"""
from typing import Any, Dict, List, Tuple
import yaml


def _load_schema(path: str = "config/preset_schema.yaml") -> Dict[str, Any]:
    with open(path, "r") as f:
        return yaml.safe_load(f)


def validate_request(request: Dict[str, Any]) -> List[str]:
    """Validate a Python request dict against schema contract.
    Returns a list of issues (empty if OK).
    """
    issues: List[str] = []
    try:
        schema = _load_schema()
    except Exception as e:
        return [f"Schema load failed: {e}"]

    contract = schema.get("contract", {}).get("request", {})

    def check_range(name: str, val: Any, lo: float, hi: float):
        if not isinstance(val, (int, float)):
            issues.append(f"{name} not numeric: {val}")
        elif not (lo <= float(val) <= hi):
            issues.append(f"{name} out of range [{lo},{hi}]: {val}")

    if "prompt" not in request:
        issues.append("prompt is required")

    ctx = request.get("context", {})
    if "tempo" in ctx:
        tempo_spec = contract["context"]["tempo"]
        check_range("context.tempo", ctx["tempo"], tempo_spec["min"], tempo_spec["max"]) 
    if "key" in ctx:
        k = ctx["key"]
        if not isinstance(k, int) or not (0 <= k <= 11):
            issues.append(f"context.key must be integer [0,11], got {k}")

    cons = request.get("constraints", {})
    if "maxCPU" in cons:
        check_range("constraints.maxCPU", cons["maxCPU"], 0.0, 1.0)
    if "maxLatency" in cons:
        check_range("constraints.maxLatency", cons["maxLatency"], 1.0, 100.0)
    if "lufsTarget" in cons:
        check_range("constraints.lufsTarget", cons["lufsTarget"], -36.0, 0.0)
    if "truePeakLimit" in cons:
        check_range("constraints.truePeakLimit", cons["truePeakLimit"], -12.0, 0.0)

    return issues
