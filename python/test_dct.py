import argparse
import os
import subprocess
import sys
import numpy as np

try:
    from scipy.fft import dct as sp_dct, idct as sp_idct
    HAVE_SCIPY = True
except Exception:
    HAVE_SCIPY = False

def maybe_skip(msg):
    print(msg)
    sys.exit(77)

def run_dump(bin_path, n, t, direction, x):
    import tempfile
    with tempfile.NamedTemporaryFile(mode='w', delete=False) as f:
        for v in x:
            f.write(f"{float(v)}\n")
        fname = f.name
    try:
        cmd = [bin_path, "--type", str(t), "--dir", direction, "-n", str(n), "--infile", fname]
        out = subprocess.check_output(cmd, text=True)
        y = np.array([float(s) for s in out.strip().splitlines()], dtype=float)
        return y
    finally:
        try:
            os.unlink(fname)
        except Exception:
            pass

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dct-bin', required=True)
    args = ap.parse_args()
    if not HAVE_SCIPY:
        maybe_skip("scipy not available; skipping DCT test")
    rtol = float(os.environ.get('VV_PY_RTOL') or 1e-6)
    atol = float(os.environ.get('VV_PY_ATOL') or 1e-6)

    rng = np.random.default_rng(0)
    for n in (7, 8, 63, 64, 257):
        x = rng.standard_normal(n)
        # DCT-II forward then inverse via DCT-III
        y = run_dump(args.dct_bin, n, 2, 'fwd', x)
        xr = run_dump(args.dct_bin, n, 2, 'inv', y)
        # Reference roundtrip comparison
        np.testing.assert_allclose(xr, x, rtol=rtol, atol=1e-4)

        # DCT-IV roundtrip
        y4 = run_dump(args.dct_bin, n, 4, 'fwd', x)
        xr4 = run_dump(args.dct_bin, n, 4, 'inv', y4)
        np.testing.assert_allclose(xr4, x, rtol=rtol, atol=1e-4)

    print("python DCT tests passed")

if __name__ == '__main__':
    main()
