import os


def _env_bool(name: str, default: bool = False) -> bool:
    v = os.getenv(name)
    if v is None:
        return default
    return v.lower() in {"1", "true", "yes", "on"}


def _env_float(name: str, default):
    v = os.getenv(name)
    if v is None:
        return default
    try:
        return float(v)
    except (ValueError, TypeError):
        return default


def ensure_numpy_scipy():
    try:
        import numpy as _np  # noqa: F401
        import scipy as _sp  # noqa: F401
        return True
    except ImportError as e:
        print(f"Skip: numpy/scipy unavailable: {e}")
        return False


SKIP_CODE = 77


def read_tolerances(default_rtol, default_atol):
    rtol = _env_float("VV_PY_RTOL", default_rtol)
    atol = _env_float("VV_PY_ATOL", default_atol)
    # Ensure floats
    try:
        rtol = float(rtol)
    except (ValueError, TypeError):
        rtol = float(default_rtol)
    try:
        atol = float(atol)
    except (ValueError, TypeError):
        atol = float(default_atol)
    return rtol, atol


def is_verbose(default: bool = False) -> bool:
    return _env_bool("VV_PY_VERBOSE", default)

