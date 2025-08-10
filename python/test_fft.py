#!/usr/bin/env python3
import argparse
import sys

from common import ensure_numpy_scipy, SKIP_CODE

if not ensure_numpy_scipy():
    sys.exit(SKIP_CODE)

import numpy as np
from numpy.testing import assert_allclose


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
    parser.add_argument('--fft-bin', required=True)
    parser.add_argument('-n', type=int, default=16)
    parser.add_argument('--rtol', type=float, default=5e-5)
    parser.add_argument('--atol', type=float, default=5e-5)
    args = parser.parse_args()

    rng = np.random.default_rng(0)

    # C2C forward
    x = rng.random(args.n) + 1j * rng.random(args.n)
    infile = 'tmp_c2c_in.txt'
    np.savetxt(
        infile,
        np.c_[x.real.astype(np.float32), x.imag.astype(np.float32)],
        fmt='%.8g',
        delimiter=',',
    )
    out_lines = run_cmd([
        args.fft_bin, '--type', 'c2c', '--dir', 'fwd', '-n', str(args.n),
        '--infile', infile,
    ])
    y = parse_complex_lines(out_lines)
    y_ref = np.fft.fft(x)
    assert_allclose(y, y_ref, rtol=args.rtol, atol=args.atol)

    # R2C forward
    xr = rng.random(args.n)
    infile = 'tmp_r2c_in.txt'
    np.savetxt(infile, xr.astype(np.float32), fmt='%.8g')
    out_lines = run_cmd([
        args.fft_bin, '--type', 'r2c', '--dir', 'fwd', '-n', str(args.n),
        '--infile', infile,
    ])
    y = parse_complex_lines(out_lines)
    y_ref = np.fft.rfft(xr)
    assert_allclose(y, y_ref, rtol=args.rtol, atol=args.atol)

    # C2R inverse (assumes vv-dsp scales inverse by 1/n)
    X = np.fft.rfft(xr)
    infile = 'tmp_c2r_in.txt'
    np.savetxt(
        infile,
        np.c_[X.real.astype(np.float32), X.imag.astype(np.float32)],
        fmt='%.8g',
        delimiter=',',
    )
    out_lines = run_cmd([
        args.fft_bin, '--type', 'c2r', '--dir', 'inv', '-n', str(args.n),
        '--infile', infile,
    ])
    y = np.array(list(map(float, out_lines)))
    y_ref = np.fft.irfft(X, n=args.n)
    assert_allclose(y, y_ref, rtol=args.rtol, atol=args.atol)

    print('FFT tests passed')
    return 0


if __name__ == '__main__':
    sys.exit(main())
