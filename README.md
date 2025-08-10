# vv-dsp

Digital Signal Processing (DSP) building blocks in C. Modular, portable, and testable.

## Build

```sh
cmake -S . -B build
cmake --build build -j
```

## Test

```sh
ctest --test-dir build --output-on-failure
```

## Options

- `-DVV_DSP_BUILD_TESTS=ON|OFF` (default ON)
- `-DVV_DSP_USE_SIMD=ON|OFF` (default OFF)
- `-DVV_DSP_BACKEND_FFT=none|fftw|kissfft` (default `none`)
- `-DVV_DSP_SINGLE_FILE=ON|OFF` (default OFF)
- `-DVERIFY_WITH_PYTHON=ON|OFF` (default OFF) — Enable NumPy/SciPy-based cross-validation tests

## Python-based validation (optional)

The Python validation suite compares vv-dsp CLI outputs against NumPy/SciPy references.

1. Install dependencies (recommended in a virtual environment):

```sh
python3 -m venv .venv  # if available; on Debian/Ubuntu: sudo apt install python3-venv
source .venv/bin/activate
pip install -U pip
pip install -r python/requirements.txt
```

1. Configure with validation enabled and build:

```sh
cmake -S . -B build -DVERIFY_WITH_PYTHON=ON -DVV_DSP_BUILD_TESTS=ON
cmake --build build -j
```

1. Run only validation tests:

```sh
ctest --test-dir build -L validation --output-on-failure
```

Notes:

- If NumPy/SciPy is not importable, CMake will skip registering validation tests. If tests run but Python scripts exit with code 77, CTest will mark them as SKIP.
- When building with sanitizers (`-DVV_DSP_ENABLE_ASAN=ON` or `-DVV_DSP_ENABLE_UBSAN=ON`), the Python tests set friendly defaults (e.g., `ASAN_OPTIONS=detect_leaks=0`) to avoid noise from Python's allocator. Sanitizers remain enabled for vv-dsp binaries.

Failure interpretation:

- Use `--output-on-failure` with ctest to see the Python traceback and NumPy's `assert_allclose` report (it prints mismatched elements and max absolute/relative error).
- CTest logs are under `build/Testing/Temporary/LastTest.log`.
- Minor numeric drift can occur; adjust `--rtol/--atol` in Python scripts if needed for specific investigations, but keep repo defaults for CI.

### CI pipeline (conceptual)

An example CI job (GitHub Actions, Azure Pipelines, etc.) would:

1. Checkout sources and install build toolchain (CMake, compiler).
1. Set up Python 3 and install deps: `pip install -r python/requirements.txt`.
1. Configure: `cmake -S . -B build -DVERIFY_WITH_PYTHON=ON -DVV_DSP_BUILD_TESTS=ON`.
1. Build: `cmake --build build -j`.
1. Run: `ctest --test-dir build -L validation --output-on-failure`.
1. Optionally run remaining C tests: `ctest --test-dir build --output-on-failure`.

## License

MIT
