#!/usr/bin/env python3
import os
import sys
import subprocess
import numpy as np
from numpy.testing import assert_allclose
from common import ensure_numpy_scipy, SKIP_CODE, read_tolerances

if not ensure_numpy_scipy():
    sys.exit(SKIP_CODE)

rtol, atol = read_tolerances(1e-4, 1e-4)

# Generate a test signal
rng = np.random.default_rng(0)
x = rng.standard_normal(128).astype(np.float64)

# Reference autocorrelation (biased)
# match our definition: r[k] = mean of x[i]*x[i+k] over i
ref = np.array([np.mean(x[:len(x)-k]*x[k:]) for k in range(len(x))])

# Call CLI
bin_path = os.path.join(os.path.dirname(__file__), '..', 'tools', 'vv_dsp_dump_stats')
if not os.path.exists(bin_path):
    # When run via CTest from build dir, tools are in build/tools
    bin_path = os.path.join(os.path.dirname(__file__), '..', 'build', 'tools', 'vv_dsp_dump_stats')

proc = subprocess.run([bin_path, 'autocorr', str(len(x)), '0'],
                      input='\n'.join(str(float(v)) for v in x).encode(),
                      stdout=subprocess.PIPE, stderr=subprocess.PIPE)
if proc.returncode != 0:
    print(proc.stderr.decode(), file=sys.stderr)
    sys.exit(proc.returncode)

y = np.fromstring(proc.stdout.decode(), sep='\n')

assert_allclose(y, ref.astype(y.dtype), rtol=rtol, atol=atol)
print('test_stats: OK')
