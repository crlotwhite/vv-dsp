# VV-DSP

A comprehensive, high-performance C99 digital signal processing library designed for portability, modularity, and real-time applications.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C99](https://img.shields.io/badge/std-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Documentation](https://github.com/crlotwhite/vv-dsp/actions/workflows/docs.yml/badge.svg)](https://github.com/crlotwhite/vv-dsp/actions/workflows/docs.yml)
[![API Docs](https://img.shields.io/badge/API-Documentation-blue)](https://crlotwhite.github.io/vv-dsp/)

## Overview

VV-DSP is a production-ready digital signal processing library offering:

- **🔄 Spectral Analysis**: Multiple FFT backends (KissFFT, FFTW, FFTS), STFT, DCT, CZT, Hilbert transforms
- **🎛️ Digital Filters**: FIR, IIR, Savitzky-Golay smoothing, Butterworth, Chebyshev filters
- **📈 Sample Rate Conversion**: High-quality resampling and interpolation algorithms
- **📊 Signal Processing**: Comprehensive windowing functions, envelope detection, feature extraction
- **🧮 Mathematical Operations**: SIMD-optimized vectorized math, complex numbers, statistics, linear algebra
- **🛡️ Numerical Stability**: Configurable NaN/Inf handling policies, denormal flushing for robust processing
- **⚡ Performance**: Optional SIMD optimizations, multiple precision backends, real-time capable

## Quick Start

### Installation

Choose your preferred integration method:

**[📖 Complete Integration Guide](docs/integration.md)** - Detailed instructions for all methods

#### vcpkg (Recommended)

```bash
vcpkg install vv-dsp
```

#### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(vv-dsp
  GIT_REPOSITORY https://github.com/crlotwhite/vv-dsp.git
  GIT_TAG        v0.1.0)
FetchContent_MakeAvailable(vv-dsp)
target_link_libraries(your_target PRIVATE vv-dsp)
```

### Build from Source

```bash
cmake -S . -B build
cmake --build build -j
```

### Run Tests

```bash
ctest --test-dir build --output-on-failure
```

### Generate Documentation

```bash
doxygen Doxyfile
# Open docs/html/index.html in your browser
```

**📖 [View Online Documentation](https://crlotwhite.github.io/vv-dsp/)** - Auto-updated from main branch

## Build Configuration

VV-DSP supports extensive configuration through CMake options:

### Core Options

- **`VV_DSP_BUILD_TESTS`** (default: ON) — Build comprehensive test suite
- **`VV_DSP_BUILD_EXAMPLES`** (default: ON) — Build usage examples
- **`VV_DSP_BUILD_BENCHMARKS`** (default: OFF) — Build performance benchmarks
- **`VV_DSP_USE_SIMD`** (default: OFF) — Enable SIMD optimizations
- **`VV_DSP_SINGLE_FILE`** (default: OFF) — Generate single-header build

### FFT Backend Selection

VV-DSP supports multiple FFT implementations for optimal performance:

- **`VV_DSP_WITH_KISSFFT`** (default: ON) — Lightweight, always available
- **`VV_DSP_WITH_FFTW`** (default: OFF) — High-performance FFTW3 backend
- **`VV_DSP_WITH_FFTS`** (default: OFF) — ARM-optimized FFTS backend
- **`VV_DSP_BACKEND_FFT`** (default: "kissfft") — Default backend selection

**Example with multiple backends:**

```bash
cmake -S . -B build \
    -DVV_DSP_WITH_FFTW=ON \
    -DVV_DSP_WITH_FFTS=ON \
    -DVV_DSP_BACKEND_FFT=fftw
```

### Python Cross-Validation

VV-DSP includes a comprehensive Python validation suite for enhanced testing confidence:

#### Installation & Setup

1. **Install Python dependencies:**

```bash
python3 -m venv .venv
source .venv/bin/activate  # Windows: .venv\Scripts\activate
pip install -r python/requirements.txt
```

1. **Configure and build with validation:**

```bash
cmake -S . -B build -DVERIFY_WITH_PYTHON=ON
cmake --build build -j
```

1. **Run validation tests:**

```bash
# Validation tests only
ctest --test-dir build -L validation --output-on-failure

# All tests including validation
ctest --test-dir build --output-on-failure
```

#### Configuration Options

- **`VERIFY_WITH_PYTHON`** (default: OFF) — Enable NumPy/SciPy cross-validation
- **`VV_PY_VERBOSE`** (default: OFF) — Enable verbose Python test output
- **`VV_PY_RTOL`** — Override relative tolerance (e.g., "5e-5")
- **`VV_PY_ATOL`** — Override absolute tolerance (e.g., "5e-5")

### Performance Optimizations

#### Mathematical Approximations

- **`VV_DSP_USE_FASTAPPROX`** (default: OFF) — Fast math via [fastapprox](https://github.com/romeric/fastapprox)
- **`VV_DSP_USE_MATH_APPROX`** (default: OFF) — DSP-optimized math via [math_approx](https://github.com/Chowdhury-DSP/math_approx)
- **`VV_DSP_USE_EIGEN`** (default: OFF) — [Eigen](https://eigen.tuxfamily.org/) linear algebra integration

#### High-Performance Build Example

```bash
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DVV_DSP_USE_SIMD=ON \
    -DVV_DSP_USE_MATH_APPROX=ON \
    -DVV_DSP_USE_FASTAPPROX=ON \
    -DVV_DSP_WITH_FFTW=ON \
    -DVV_DSP_BACKEND_FFT=fftw
cmake --build build -j
```

**Performance Characteristics** (Release builds):

- Vectorized math: ~3x speedup with SIMD
- Trigonometric functions: Significant improvements with approximation libraries
- Window operations: Variable speedup depending on size and type
- Accuracy maintained: All optimizations preserve required precision (error < 6e-8)

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

    // Generate test signal with some noise
    for (size_t i = 0; i < N; i++) {
        signal[i] = sin(2.0 * M_PI * 1000.0 * i / 8000.0) +
                   0.1 * sin(2.0 * M_PI * 3000.0 * i / 8000.0);
    }

    // Apply Hann window for spectral analysis
    vv_dsp_real windowed[N];
    vv_dsp_window_hann(signal, windowed, N);

    // Compute FFT using the configured backend
    vv_dsp_cpx fft_input[N], fft_output[N];
    for (size_t i = 0; i < N; i++) {
        fft_input[i] = vv_dsp_cpx_make(windowed[i], 0.0);
    }

    vv_dsp_result_e result = vv_dsp_fft_c2c(fft_input, fft_output, N, VV_DSP_FFT_FORWARD);
    if (result != VV_DSP_SUCCESS) {
        printf("FFT failed with error: %d\n", result);
        return 1;
    }

    // Calculate magnitude spectrum
    vv_dsp_real magnitude[N];
    for (size_t i = 0; i < N; i++) {
        magnitude[i] = vv_dsp_cpx_abs(fft_output[i]);
    }

    // Find peak frequency
    size_t peak_bin = 0;
    vv_dsp_real peak_mag = 0.0;
    for (size_t i = 1; i < N/2; i++) {  // Only check positive frequencies
        if (magnitude[i] > peak_mag) {
            peak_mag = magnitude[i];
            peak_bin = i;
        }
    }

    double peak_freq = (double)peak_bin * 8000.0 / N;
    printf("Peak frequency: %.1f Hz (magnitude: %.2f)\n", peak_freq, peak_mag);

    // Apply Savitzky-Golay smoothing filter
    vv_dsp_real smoothed[N];
    result = vv_dsp_savgol(signal, smoothed, N, 5, 3);  // window=5, polynomial=3
    if (result == VV_DSP_SUCCESS) {
        printf("Savitzky-Golay smoothing applied successfully\n");
    }

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

We welcome contributions! Here's how to get involved:

### Development Setup

1. **Fork and clone** the repository
2. **Install dependencies** (Python validation, etc.)
3. **Configure development build:**

```bash
cmake -S . -B build \
    -DVV_DSP_BUILD_TESTS=ON \
    -DVV_DSP_BUILD_EXAMPLES=ON \
    -DVERIFY_WITH_PYTHON=ON \
    -DCMAKE_BUILD_TYPE=Debug
```

### Contribution Guidelines

- **🎯 Code Style**: Follow existing C99 conventions and naming patterns
- **📝 Documentation**: Add comprehensive Doxygen comments for public APIs
- **🧪 Testing**: Include unit tests and update Python validation when applicable
- **🔍 Review**: Ensure all tests pass and consider edge cases
- **📋 Integration**: Update integration guides for new features

### Areas for Contribution

- **New Algorithms**: Additional DSP algorithms and filters
- **Performance**: SIMD optimizations and algorithm improvements
- **Platform Support**: Testing and fixes for additional platforms
- **Documentation**: Examples, tutorials, and API improvements
- **Bindings**: Language bindings (Python, Rust, etc.)

For detailed guidelines, see [`DOCUMENTATION.md`](DOCUMENTATION.md) and the [Integration Guide](docs/integration.md).

## Architecture & Design

VV-DSP follows these core design principles:

### 🧩 Modularity

- **Domain Separation**: Clear boundaries between spectral, filter, and math modules
- **Optional Dependencies**: Core functionality works standalone; optimizations are additive
- **Plugin Architecture**: Swappable FFT backends and math implementations

### 🌐 Portability

- **Standard C99**: Core library uses only standard C99 features
- **Platform Adaptation**: Platform-specific optimizations through conditional compilation
- **Minimal Dependencies**: Self-contained with optional external integrations

### 🧪 Testability

- **Comprehensive Coverage**: Unit tests for all public APIs
- **Cross-Validation**: Python/NumPy reference implementations for accuracy verification
- **Multiple Platforms**: CI testing across Windows, macOS, and Linux

### ⚡ Performance

- **Zero-Copy Design**: Minimal memory allocations and data copying
- **SIMD Optimization**: Optional vectorization for compute-intensive operations
- **Backend Selection**: Choose optimal implementations at build time

### 🔧 Extensibility

- **Backend System**: Easy integration of new FFT libraries and math backends
- **Configuration System**: Fine-grained control over features and performance trade-offs
- **Future-Proof**: Designed to accommodate new algorithms and optimizations

## License

Released under the [MIT License](LICENSE). See the LICENSE file for full details.

---

**VV-DSP** - High-performance digital signal processing for modern C applications.
Built with ❤️ for the audio and signal processing community.
