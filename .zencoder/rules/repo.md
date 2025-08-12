---
description: Repository Information Overview
alwaysApply: true
---

# VV-DSP Information

## Summary
VV-DSP is a comprehensive, modular C99 digital signal processing library designed for portability, performance, and testability. It provides essential DSP operations including spectral analysis, digital filters, sample rate conversion, signal processing, feature extraction, and mathematical operations.

## Structure
- **src/**: Core library implementation organized by modules
  - **core/**: Core functionality and utilities
  - **spectral/**: FFT, STFT, DCT, CZT, Hilbert transforms
  - **filter/**: FIR, IIR, Savitzky-Golay filters
  - **resample/**: Resampling and interpolation
  - **window/**: Windowing functions
  - **envelope/**: Envelope detection
  - **features/**: MFCC and audio features
  - **adapters/**: Integration adapters
  - **audio/**: Audio I/O utilities (optional)
- **include/**: Public header files
- **tests/**: Comprehensive test suite
- **tools/**: Utility tools for DSP operations
- **python/**: Python validation scripts
- **examples/**: Example code demonstrating library usage
- **docs/**: Documentation

## Language & Runtime
**Language**: C99 (core library), C++20 (C++ bindings)
**Build System**: CMake (minimum version 3.15)
**Package Manager**: None (uses CMake's FetchContent for dependencies)

## Dependencies
**Main Dependencies**:
- KissFFT (default FFT backend, included)
- FFTW3 (optional FFT backend)
- FFTS (optional FFT backend)

**Optional Dependencies**:
- FastApprox (fast math approximations)
- Math Approx (DSP-optimized math)
- Eigen (linear algebra operations)

**Python Dependencies** (for validation):
- NumPy
- SciPy

## Build & Installation
```bash
# Basic build
cmake -S . -B build
cmake --build build -j

# With optimizations
cmake -S . -B build \
    -DVV_DSP_USE_FASTAPPROX=ON \
    -DVV_DSP_USE_MATH_APPROX=ON \
    -DVV_DSP_USE_SIMD=ON
cmake --build build -j
```

## Testing
**Framework**: CTest
**Test Location**: tests/ directory
**Naming Convention**: *_tests.c for C tests, test_cpp_*.cpp for C++ tests
**Run Command**:
```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run Python validation tests
ctest --test-dir build -L validation --output-on-failure
```

## Python Validation
**Purpose**: Cross-validation against NumPy/SciPy implementations
**Setup**:
```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r python/requirements.txt
```
**Configuration**: Enable with `-DVERIFY_WITH_PYTHON=ON` during CMake configuration

## CI/CD
**Platforms**: Ubuntu, macOS, Windows
**Configurations**:
- Regular builds
- Sanitizer builds (AddressSanitizer, UndefinedBehaviorSanitizer)
**Workflow**: Automated build, test, and validation via GitHub Actions