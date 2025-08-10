#!/usr/bin/env python3
import argparse
import os
import sys

from common import ensure_numpy_scipy, SKIP_CODE

if not ensure_numpy_scipy():
    sys.exit(SKIP_CODE)

import numpy as np
from numpy.testing import assert_allclose
from scipy.signal import lfilter


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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--fir-bin', required=True)
    parser.add_argument('--iir-bin', required=True)
    parser.add_argument('--rtol', type=float, default=3e-3)
    parser.add_argument('--atol', type=float, default=3e-3)
    parser.add_argument('-n', type=int, default=256)
    args = parser.parse_args()

    rng = np.random.default_rng(1)
    x = rng.standard_normal(args.n).astype(np.float32)

    # FIR compare with scipy firwin + lfilter
    num_taps = 33
    cutoff = 0.25
    np.savetxt('tmp_fir_in.txt', x, fmt='%.8g')
    # Dump designed coefficients to re-use here
    coeff_path = 'tmp_fir_coeffs.txt'
    coeff_lines = run_cmd([
        os.path.join(os.path.dirname(args.fir_bin), 'vv_dsp_dump_fir_coeffs'),
        '--num-taps', str(num_taps), '--cutoff', str(cutoff), '--win', 'hann',
    ])
    with open(coeff_path, 'w') as f:
        f.write("\n".join(coeff_lines) + "\n")
    y_lines = run_cmd([
        args.fir_bin, '--num-taps', str(num_taps), '--cutoff', str(cutoff),
        '--win', 'hann', '--n', str(args.n), '--infile', 'tmp_fir_in.txt',
    ])
    y = np.array(list(map(float, y_lines)))

    h = np.loadtxt(coeff_path)
    y_ref = lfilter(h, [1.0], x)
    assert_allclose(y, y_ref, rtol=args.rtol, atol=args.atol)

    # IIR: one-pole lowpass in DF2T form
    # y[n] = b0*x[n] + ... - a1*y[n-1] - a2*y[n-2]
    a1 = -0.9
    a2 = 0.0
    b0 = 0.1
    b1 = 0.0
    b2 = 0.0
    np.savetxt('tmp_iir_in.txt', x, fmt='%.8g')
    y_lines = run_cmd([
        args.iir_bin, '--b0', str(b0), '--b1', str(b1), '--b2', str(b2),
        '--a1', str(a1), '--a2', str(a2), '--n', str(args.n), '--infile',
        'tmp_iir_in.txt',
    ])
    y = np.array(list(map(float, y_lines)))

    y_ref = lfilter([b0, b1, b2], [1.0, -a1, -a2], x)
    assert_allclose(y, y_ref, rtol=args.rtol, atol=args.atol)

    print('Filter tests passed')
    return 0


if __name__ == '__main__':
    sys.exit(main())
