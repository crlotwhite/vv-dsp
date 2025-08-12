# VV-DSP

A comprehensive, modular C99 digital signal processing library designed for portability, performance, and testability.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C99](https://img.shields.io/badge/std-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Documentation](https://github.com/crlotwhite/vv-dsp/actions/workflows/docs.yml/badge.svg)](https://github.com/crlotwhite/vv-dsp/actions/workflows/docs.yml)
[![API Docs](https://img.shields.io/badge/API-Documentation-blue)](https://crlotwhite.github.io/vv-dsp/)

## Overview

VV-DSP provides essential digital signal processing operations including:

- **Spectral Analysis**: FFT, STFT, DCT, CZT, Hilbert transforms
- **Digital Filters**: FIR, IIR, Savitzky-Golay smoothing filters
- **Sample Rate Conversion**: Resampling and interpolation
- **Signal Processing**: Windowing functions, envelope detection
- **Feature Extraction**: MFCC and other audio features
- **Mathematical Operations**: Complex numbers, statistics, linear algebra
- **Numerical Stability**: Configurable NaN/Inf handling policies for robust processing

## Quick Start

### Build

```sh
cmake -S . -B build
cmake --build build -j
```

### Test

```sh
ctest --test-dir build --output-on-failure
```

### Documentation

Generate API documentation using Doxygen:

```sh
doxygen Doxyfile
# Open docs/html/index.html in your browser
```

**📖 [View Online Documentation](https://crlotwhite.github.io/vv-dsp/)** - Auto-updated from main branch

## Build Configuration

The library supports various configuration options through CMake:

### Basic Options

- `-DVV_DSP_BUILD_TESTS=ON|OFF` (default ON) — Build test suite
- `-DVV_DSP_USE_SIMD=ON|OFF` (default OFF) — Enable SIMD optimizations
- `-DVV_DSP_BACKEND_FFT=none|fftw|kissfft` (default `none`) — FFT backend
- `-DVV_DSP_SINGLE_FILE=ON|OFF` (default OFF) — Generate single-header build

### Python Validation

- `-DVERIFY_WITH_PYTHON=ON|OFF` (default OFF) — Enable NumPy/SciPy cross-validation
  - Controls for validation tests:
    - `-DVV_PY_VERBOSE=ON` to increase Python test logging
    - `-DVV_PY_RTOL=5e-5` and/or `-DVV_PY_ATOL=5e-5` to override tolerances

### Performance Dependencies (Optional)

VV-DSP supports optional integration with external libraries for enhanced performance:

- `-DVV_DSP_USE_FASTAPPROX=ON|OFF` (default OFF) — Fast math approximations via [fastapprox](https://github.com/romeric/fastapprox)
- `-DVV_DSP_USE_MATH_APPROX=ON|OFF` (default OFF) — DSP-optimized math via [math_approx](https://github.com/Chowdhury-DSP/math_approx)
- `-DVV_DSP_USE_EIGEN=ON|OFF` (default OFF) — [Eigen](https://eigen.tuxfamily.org/) linear algebra library

**Example with performance optimizations:**

```sh
cmake -S . -B build \
    -DVV_DSP_USE_FASTAPPROX=ON \
    -DVV_DSP_USE_MATH_APPROX=ON \
    -DVV_DSP_USE_EIGEN=ON \
    -DVV_DSP_USE_SIMD=ON \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

#### Math Approximation Library Usage

The `-DVV_DSP_USE_MATH_APPROX=ON` flag enables optimized math function approximations for DSP operations. This provides a uniform interface for math operations while allowing runtime selection of precision vs. performance trade-offs.

**Integration**: All math-intensive operations automatically use the optimized functions when enabled:

- `VV_DSP_SIN`, `VV_DSP_COS`, `VV_DSP_TAN` - Trigonometric functions
- `VV_DSP_EXP`, `VV_DSP_LOG` - Exponential and logarithmic functions
- `VV_DSP_POW`, `VV_DSP_SQRT`, `VV_DSP_ATAN2` - Power and root functions

**Performance Characteristics** (Release build, ARM64):

- Trigonometric functions: ~3x speedup with Eigen vectorization
- Window operations: Variable speedup depending on size
- Complex operations: Modest 1.1-1.2x improvements
- Accuracy: All functions maintain floating-point precision (error < 6e-8)

**Note**: Performance optimizations require Release builds (`-DCMAKE_BUILD_TYPE=Release`) to realize SIMD benefits. Debug builds may show reduced performance due to disabled optimizations.

## Python Validation Suite

The Python validation suite provides cross-validation of vv-dsp outputs against NumPy/SciPy references for enhanced testing confidence.

### Setup

1. **Install Python dependencies** (recommended in a virtual environment):

```sh
python3 -m venv .venv  # if available; on Debian/Ubuntu: sudo apt install python3-venv
source .venv/bin/activate
pip install -U pip
pip install -r python/requirements.txt
```

1. **Configure and build with validation enabled:**

```sh
cmake -S . -B build -DVERIFY_WITH_PYTHON=ON -DVV_DSP_BUILD_TESTS=ON
cmake --build build -j
```

1. **Run validation tests:**

```sh
# Run only validation tests
ctest --test-dir build -L validation --output-on-failure

# Run all tests
ctest --test-dir build --output-on-failure
```

### Configuration Notes

- If NumPy/SciPy is not importable, CMake will skip registering validation tests
- When tests run but Python scripts exit with code 77, CTest marks them as SKIP
- Sanitizer builds (`-DVV_DSP_ENABLE_ASAN=ON`, `-DVV_DSP_ENABLE_UBSAN=ON`) set friendly defaults to avoid noise from Python's allocator
- Tolerances and verbosity can be tuned via CMake cache variables (`VV_PY_RTOL`, `VV_PY_ATOL`, `VV_PY_VERBOSE`)

### Troubleshooting

**Interpreting failures:**

- Use `--output-on-failure` with ctest to see Python tracebacks and NumPy's `assert_allclose` reports
- CTest logs are available under `build/Testing/Temporary/LastTest.log`
- Minor numeric drift may occur; adjust tolerances in Python scripts if needed for specific investigations

**Example CI Pipeline:**

1. Install build toolchain (CMake, compiler) and Python 3
1. Install dependencies: `pip install -r python/requirements.txt`
1. Configure: `cmake -S . -B build -DVERIFY_WITH_PYTHON=ON -DVV_DSP_BUILD_TESTS=ON`
1. Build: `cmake --build build -j`
1. Test: `ctest --test-dir build -L validation --output-on-failure`

## NaN/Inf Handling

VV-DSP provides configurable policies for handling NaN (Not-a-Number) and Infinity values to ensure numerical stability and prevent undefined behavior in production environments.

### Policy Configuration

```c
#include "vv_dsp/core/nan_policy.h"

// Set global policy (thread-local if threading support is enabled)
vv_dsp_set_nan_policy(VV_DSP_NAN_POLICY_IGNORE);

// Get current policy
vv_dsp_nan_policy_e policy = vv_dsp_get_nan_policy();
```

### Available Policies

- **`VV_DSP_NAN_POLICY_PROPAGATE`** (default): Let NaN/Inf values pass through calculations
- **`VV_DSP_NAN_POLICY_IGNORE`**: Replace NaN/Inf with neutral values (0.0)
- **`VV_DSP_NAN_POLICY_ERROR`**: Return error code immediately upon detecting NaN/Inf
- **`VV_DSP_NAN_POLICY_CLAMP`**: Replace +/-Inf with max/min finite values, NaN with 0.0

### Error Handling

When the policy is set to `VV_DSP_NAN_POLICY_ERROR`, functions return `VV_DSP_ERROR_NAN_INF` when NaN or Infinity values are detected in input or output data.

### Supported Functions

The NaN/Inf policy is automatically applied to:

- Savitzky-Golay filters (`vv_dsp_savgol`)
- DCT transforms (`vv_dsp_dct_execute`, convenience functions)
- All module functions that process floating-point arrays

## API Example

```c
#include "vv_dsp/vv_dsp.h"
#include <stdio.h>
#include <math.h>

int main() {
    // Create a test signal: 1 kHz sine wave sampled at 8 kHz
    const size_t N = 256;
    vv_dsp_real signal[N];

    for (size_t i = 0; i < N; i++) {
        signal[i] = sin(2.0 * M_PI * 1000.0 * i / 8000.0);
    }

    // Apply Hann window
    vv_dsp_real windowed[N];
    vv_dsp_window_hann(signal, windowed, N);

    // Compute FFT
    vv_dsp_cpx fft_input[N], fft_output[N];
    for (size_t i = 0; i < N; i++) {
        fft_input[i] = vv_dsp_cpx_make(windowed[i], 0.0);
    }
    vv_dsp_fft_c2c(fft_input, fft_output, N, VV_DSP_FFT_FORWARD);

    // Calculate magnitude spectrum
    vv_dsp_real magnitude[N];
    for (size_t i = 0; i < N; i++) {
        magnitude[i] = vv_dsp_cpx_abs(fft_output[i]);
    }

    printf("Peak magnitude at bin %zu: %.2f\n", 32, magnitude[32]); // ~1kHz bin
    return 0;
}
```

## API Documentation

### Doxygen API Reference

Complete API documentation is available through Doxygen:

**🌐 [Online Documentation](https://crlotwhite.github.io/vv-dsp/)** (automatically updated)

**Local Generation:**

```sh
# Generate documentation
doxygen Doxyfile

# View in browser
open docs/html/index.html  # macOS
xdg-open docs/html/index.html  # Linux
```

The generated documentation includes:

- **API Reference**: Detailed function documentation with parameters and examples
- **Module Organization**: Functions grouped by category (Core, Spectral, Filter, etc.)
- **Usage Examples**: Code snippets demonstrating common operations
- **Cross-References**: Links between related functions and types

### Manual Documentation

- [`DOCUMENTATION.md`](DOCUMENTATION.md) - Documentation generation and contribution guidelines
- [`.github/PAGES_SETUP.md`](.github/PAGES_SETUP.md) - GitHub Pages automatic deployment setup
- Source code headers contain comprehensive inline documentation

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Code Style**: Follow the existing C99 style and naming conventions
1. **Documentation**: Add Doxygen comments for all public functions
1. **Testing**: Include tests for new features and bug fixes
1. **Cross-Validation**: Update Python validation scripts when applicable

For detailed documentation guidelines, see [`DOCUMENTATION.md`](DOCUMENTATION.md).

## Architecture

VV-DSP is designed with several key principles:

- **Modularity**: Clean separation between different DSP domains
- **Portability**: Standard C99 with optional platform-specific optimizations
- **Testability**: Comprehensive test coverage with cross-validation
- **Performance**: Configurable SIMD and external library integration
- **Extensibility**: Plugin architecture for FFT backends and math libraries

## License

MIT
