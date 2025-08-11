#!/usr/bin/env python3
import argparse
import sys

from common import ensure_numpy_scipy, SKIP_CODE, read_tolerances, is_verbose

if not ensure_numpy_scipy():
    sys.exit(SKIP_CODE)

import numpy as np
from numpy.testing import assert_allclose
from scipy.signal import czt


def run_cmd(cmd):
    import subprocess
    p = subprocess.run(
        cmd,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return p.stdout.strip().splitlines()


def parse_complex_lines(lines):
    arr = np.array(
        [list(map(float, ln.split(','))) for ln in lines], dtype=float
    )
    return arr[:, 0] + 1j * arr[:, 1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--czt-bin', required=True)
    parser.add_argument('-n', type=int, default=32)
    parser.add_argument('-m', type=int, default=32)
    parser.add_argument('--rtol', type=float, default=2e-4)
    parser.add_argument('--atol', type=float, default=2e-4)
    args = parser.parse_args()

    rng = np.random.default_rng(0)
    rtol, atol = read_tolerances(args.rtol, args.atol)
    verbose = is_verbose()

    # random complex signal
    x = rng.standard_normal(args.n) + 1j * rng.standard_normal(args.n)
    infile = 'tmp_czt_in.txt'
    np.savetxt(
        infile,
        np.c_[x.real.astype(np.float32), x.imag.astype(np.float32)],
        fmt='%.8g',
        delimiter=',',
    )

    # choose parameters equivalent to DFT: A=1, W=e^{-j 2pi/N}, M=N
    ang = -2 * np.pi / args.n
    Wre, Wim = np.cos(ang), np.sin(ang)
    Are, Aim = 1.0, 0.0

    out_lines = run_cmd([
        args.czt_bin,
        '--N', str(args.n), '--M', str(args.m),
        '--Wre', f'{Wre}', '--Wim', f'{Wim}',
        '--Are', f'{Are}', '--Aim', f'{Aim}',
        '--infile', infile,
        '--complex'
    ])
    y = parse_complex_lines(out_lines)

    # SciPy reference
    y_ref = czt(x, m=args.m, w=np.exp(1j * ang), a=1.0)

    try:
        assert_allclose(y, y_ref, rtol=rtol, atol=atol)
    except AssertionError as e:
        if verbose:
            print('CZT mismatch:', e)
        raise

    # Also test zoomed range around a target frequency on real input
    fs = 48000.0
    f0 = 1000.0
    t = np.arange(args.n) / fs
    xr = np.cos(2*np.pi*f0*t)
    M = 64
    f_start, f_end = 800.0, 1200.0
    delta = (f_end - f_start) / M
    W = np.exp(-1j * 2*np.pi*delta/fs)
    A = np.exp(-1j * 2*np.pi*f_start/fs)

    np.savetxt('tmp_czt_in_real.txt', xr.astype(np.float32), fmt='%.8g')
    out_lines = run_cmd([
        args.czt_bin,
        '--N', str(args.n), '--M', str(M),
        '--Wre', f'{W.real}', '--Wim', f'{W.imag}',
        '--Are', f'{A.real}', '--Aim', f'{A.imag}',
        '--infile', 'tmp_czt_in_real.txt'
    ])
    y2 = parse_complex_lines(out_lines)
    y2_ref = czt(xr, m=M, w=W, a=A)
    try:
        assert_allclose(y2, y2_ref, rtol=rtol, atol=atol)
    except AssertionError as e:
        if verbose:
            print('CZT zoom mismatch:', e)
        raise

    print('CZT tests passed')
    return 0


if __name__ == '__main__':
    sys.exit(main())
