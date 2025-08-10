def ensure_numpy_scipy():
    try:
        import numpy as _np  # noqa: F401
        import scipy as _sp  # noqa: F401
        return True
    except ImportError as e:
        print(f"Skip: numpy/scipy unavailable: {e}")
        return False


SKIP_CODE = 77

