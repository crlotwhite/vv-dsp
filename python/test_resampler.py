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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--resample-bin', required=True)
    parser.add_argument('--rtol', type=float, default=5e-2)
    parser.add_argument('--atol', type=float, default=5e-2)
    parser.add_argument('-n', type=int, default=400)
    args = parser.parse_args()

    rng = np.random.default_rng(3)
    x = rng.standard_normal(args.n).astype(np.float32)

    # Up by 2, linear
    np.savetxt('tmp_resamp_in.txt', x, fmt='%.8g')
    y_lines = run_cmd([
        args.resample_bin, '--num', '2', '--den', '1', '--quality', 'linear',
        '--infile', 'tmp_resamp_in.txt',
    ])
    y = np.array(list(map(float, y_lines)))
    # Reference: linear interpolation
    # Library maps endpoints: expect = floor((n-1)*ratio)+1
    expect = int(np.floor((len(x) - 1) * 2.0) + 1)
    idx = np.arange(expect) / 2.0
    i0 = np.floor(idx).astype(int)
    frac = idx - i0
    i0c = np.clip(i0, 0, len(x) - 1)
    i1c = np.clip(i0 + 1, 0, len(x) - 1)
    ref = (1 - frac) * x[i0c] + frac * x[i1c]
    y = np.array(list(map(float, y_lines)))
    L = min(len(ref), len(y))
    assert_allclose(y[:L], ref[:L], rtol=args.rtol, atol=args.atol)

    print('Resampler tests passed')
    return 0


if __name__ == '__main__':
    sys.exit(main())
