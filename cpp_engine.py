"""
CPP Engine adapter for Python.
Tries to use pybind11 module `aiaudio_python` to render audio; falls back to None.
"""
from typing import Dict, Any, Optional, Tuple

try:
    import numpy as np
    import aiaudio_python  # type: ignore
    _CPP_AVAILABLE = True
except Exception:  # pragma: no cover
    import numpy as np  # type: ignore
    _CPP_AVAILABLE = False


def is_available() -> bool:
    return _CPP_AVAILABLE


def _map_request(prompt: str, role: Optional[str] = None, tempo: Optional[float] = None,
                 key: Optional[int] = None, max_latency_ms: Optional[float] = None) -> Dict[str, Any]:
    req: Dict[str, Any] = {
        "prompt": prompt,
        "useSemanticSearch": True,
        "applyPolicies": True,
        "optimizeForMOO": True,
    }
    if role:
        req["role"] = role
    ctx: Dict[str, Any] = {}
    if tempo is not None:
        ctx["tempo"] = tempo
    if key is not None:
        ctx["key"] = int(key)
    if ctx:
        req["context"] = ctx
    cons: Dict[str, Any] = {}
    if max_latency_ms is not None:
        cons["maxLatency"] = float(max_latency_ms)
    if cons:
        req["constraints"] = cons
    return req


def render_sync(prompt: str, role: Optional[str] = None, tempo: Optional[float] = None,
                key: Optional[int] = None, max_latency_ms: Optional[float] = None
               ) -> Optional[Tuple["np.ndarray", Dict[str, Any]]]:
    if not _CPP_AVAILABLE:
        return None
    req = _map_request(prompt, role, tempo, key, max_latency_ms)
    audio, meta = aiaudio_python.render_sync(req)
    return audio, dict(meta)
