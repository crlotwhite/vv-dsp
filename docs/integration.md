# vv-dsp Integration Guide

This guide provides comprehensive instructions for integrating the vv-dsp digital signal processing library into your C/C++ projects using various methods.

## Table of Contents

- [Overview](#overview)
- [vcpkg Integration](#vcpkg-integration)
  - [Using vcpkg Manifest Mode (Recommended)](#using-vcpkg-manifest-mode-recommended)
  - [Using vcpkg Classic Mode](#using-vcpkg-classic-mode)
  - [Using Custom vcpkg Registry](#using-custom-vcpkg-registry)
- [CMake FetchContent Integration](#cmake-fetchcontent-integration)
- [Git Submodule Integration](#git-submodule-integration)
- [Package Features](#package-features)
- [Usage Examples](#usage-examples)
- [Troubleshooting](#troubleshooting)

## Overview

vv-dsp is a lightweight, real-time focused digital signal processing library written in C99. It provides:

- FFT/IFFT operations with multiple backends (KissFFT, FFTW, FFTS)
- FIR and IIR filtering
- Window functions (Hann, Hamming, Blackman, etc.)
- Resampling and decimation
- Spectral analysis (STFT, CZT, DCT)
- MFCC feature extraction
- Statistical functions
- Optional SIMD optimizations

## vcpkg Integration

### Using vcpkg Manifest Mode (Recommended)

1. **Create or update your project's `vcpkg.json`:**

```json
{
  "dependencies": [
    "vv-dsp"
  ]
}
```

2. **Configure your `CMakeLists.txt`:**

```cmake
cmake_minimum_required(VERSION 3.15)

project(MyProject VERSION 1.0.0 LANGUAGES C)

# Find vv-dsp package
find_package(vv-dsp CONFIG REQUIRED)

# Create your executable
add_executable(my_app main.c)

# Link against vv-dsp
target_link_libraries(my_app PRIVATE vv-dsp::vv-dsp)

# Set C standard
set_property(TARGET my_app PROPERTY C_STANDARD 99)
```

3. **Configure with vcpkg toolchain:**

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Using vcpkg Features

Enable optional features by specifying them in your `vcpkg.json`:

```json
{
  "dependencies": [
    {
      "name": "vv-dsp",
      "features": ["fftw", "simd", "audio-io"]
    }
  ]
}
```

Available features:
- `fftw`: Enable FFTW3 backend for high-performance FFT
- `ffts`: Enable FFTS backend for mobile-optimized FFT
- `simd`: Enable SIMD optimizations (SSE4.1, AVX2, NEON)
- `audio-io`: Enable WAV file I/O utilities
- `fastapprox`: Enable fast math approximations
- `math-approx`: Enable DSP-optimized math approximations
- `benchmarks`: Build performance benchmarks
- `examples`: Build example applications
- `tests`: Build test suite

### Using vcpkg Classic Mode

```bash
# Install vv-dsp with optional features
vcpkg install vv-dsp[fftw,simd]

# Use in your CMake project
find_package(vv-dsp CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE vv-dsp::vv-dsp)
```

### Using Custom vcpkg Registry

For the latest features and updates, you can use our custom vcpkg registry:

1. **Create or update your project's `vcpkg-configuration.json`:**

```json
{
  "default-registry": {
    "kind": "git",
    "repository": "https://github.com/Microsoft/vcpkg",
    "baseline": "2024.01.12"
  },
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/crlotwhite/vv-dsp-vcpkg-registry",
      "baseline": "main",
      "packages": [ "vv-dsp" ]
    }
  ]
}
```

2. **Configure your project's `vcpkg.json`:**

```json
{
  "dependencies": [
    {
      "name": "vv-dsp",
      "features": ["fftw", "simd"]
    }
  ]
}
```

3. **Use in your CMake project:**

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyProject VERSION 1.0.0 LANGUAGES C)

find_package(vv-dsp CONFIG REQUIRED)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE vv-dsp::vv-dsp)
set_property(TARGET my_app PROPERTY C_STANDARD 99)
```

4. **Build your project:**

```bash
cmake -S . -B build
cmake --build build
```

**Benefits of using the custom registry:**
- Latest features and bug fixes
- Faster updates compared to official vcpkg registry
- Direct control over port maintenance and updates
- Consistent with project development cycle

## CMake FetchContent Integration

For projects that prefer to manage dependencies directly within CMake:

```cmake
cmake_minimum_required(VERSION 3.15)

project(MyProject VERSION 1.0.0 LANGUAGES C)

include(FetchContent)

# Configure vv-dsp options before fetching
set(VV_DSP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(VV_DSP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VV_DSP_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

# Optional: Enable specific features
set(VV_DSP_USE_SIMD ON CACHE BOOL "" FORCE)
set(VV_DSP_WITH_FFTW ON CACHE BOOL "" FORCE)
set(VV_DSP_ENABLE_AUDIO_IO ON CACHE BOOL "" FORCE)

# Fetch vv-dsp
FetchContent_Declare(
  vv-dsp
  GIT_REPOSITORY https://github.com/crlotwhite/vv-dsp.git
  GIT_TAG        v0.1.0  # Use a specific version tag
)

FetchContent_MakeAvailable(vv-dsp)

# Create your executable
add_executable(my_app main.c)

# Link against vv-dsp
target_link_libraries(my_app PRIVATE vv-dsp)

# Set C standard
set_property(TARGET my_app PROPERTY C_STANDARD 99)
```

### Available CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `VV_DSP_BUILD_TESTS` | `ON` | Build test suite |
| `VV_DSP_BUILD_EXAMPLES` | `ON` | Build example applications |
| `VV_DSP_BUILD_BENCHMARKS` | `OFF` | Build performance benchmarks |
| `VV_DSP_USE_SIMD` | `OFF` | Enable SIMD optimizations |
| `VV_DSP_ENABLE_AUDIO_IO` | `OFF` | Enable WAV file I/O utilities |
| `VV_DSP_WITH_FFTW` | `OFF` | Enable FFTW3 backend |
| `VV_DSP_WITH_FFTS` | `OFF` | Enable FFTS backend |
| `VV_DSP_USE_FASTAPPROX` | `OFF` | Enable fast math approximations |
| `VV_DSP_USE_MATH_APPROX` | `OFF` | Enable DSP-optimized math approximations |

## Git Submodule Integration

For projects that want to include vv-dsp as a Git submodule:

### 1. Add vv-dsp as a submodule

```bash
# From your project root
git submodule add https://github.com/crlotwhite/vv-dsp.git third_party/vv-dsp
git submodule update --init --recursive
```

### 2. Update your `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.15)

project(MyProject VERSION 1.0.0 LANGUAGES C)

# Configure vv-dsp options
set(VV_DSP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(VV_DSP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VV_DSP_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

# Optional: Enable specific features
set(VV_DSP_USE_SIMD ON CACHE BOOL "" FORCE)

# Add vv-dsp subdirectory
add_subdirectory(third_party/vv-dsp)

# Create your executable
add_executable(my_app main.c)

# Link against vv-dsp
target_link_libraries(my_app PRIVATE vv-dsp)

# Set C standard
set_property(TARGET my_app PROPERTY C_STANDARD 99)
```

### 3. Build your project

```bash
cmake -S . -B build
cmake --build build
```

### 4. Update the submodule (when needed)

```bash
cd third_party/vv-dsp
git fetch origin
git checkout v0.1.1  # or desired version
cd ../..
git add third_party/vv-dsp
git commit -m "Update vv-dsp to v0.1.1"
```

## Package Features

### FFT Backends

vv-dsp supports multiple FFT backends:

- **KissFFT** (default): Built-in, always available, good for general use
- **FFTW3**: High-performance, best for compute-intensive applications
- **FFTS**: Optimized for mobile devices and embedded systems

```c
#include <vv_dsp/vv_dsp.h>

// Set FFT backend (optional, KissFFT is default)
vv_dsp_fft_set_backend(VV_DSP_FFT_BACKEND_FFTW);

// Create FFT plan
vv_dsp_fft_plan* plan;
vv_dsp_fft_make_plan(1024, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);

// Use the plan...
vv_dsp_fft_destroy(plan);
```

### SIMD Optimizations

When `simd` feature is enabled, vv-dsp automatically detects and uses available SIMD instructions:

- **x86/x64**: SSE4.1, AVX2, AVX512
- **ARM/AArch64**: NEON

No additional code changes are required - optimizations are applied automatically.

### Audio I/O

When `audio-io` feature is enabled:

```c
#include <vv_dsp/audio.h>

// Read WAV file
vv_dsp_wav_data* wav;
vv_dsp_wav_read("input.wav", &wav);

// Process audio data...

// Write WAV file
vv_dsp_wav_write("output.wav", wav);
vv_dsp_wav_free(wav);
```

## Usage Examples

### Basic FFT Example

```c
#include <stdio.h>
#include <vv_dsp/vv_dsp.h>

int main(void) {
    const size_t N = 1024;

    // Create FFT plan
    vv_dsp_fft_plan* plan;
    vv_dsp_status status = vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);
    if (status != VV_DSP_OK) {
        fprintf(stderr, "Failed to create FFT plan: %d\n", status);
        return 1;
    }

    // Prepare input and output buffers
    vv_dsp_cpx input[N], output[N];

    // Fill input with test signal...
    for (size_t i = 0; i < N; i++) {
        input[i].real = (vv_dsp_real)sin(2.0 * M_PI * 10.0 * i / N);  // 10 Hz signal
        input[i].imag = 0.0f;
    }

    // Execute FFT
    status = vv_dsp_fft_execute(plan, input, output);
    if (status != VV_DSP_OK) {
        fprintf(stderr, "FFT execution failed: %d\n", status);
        vv_dsp_fft_destroy(plan);
        return 1;
    }

    // Process output...
    printf("FFT completed successfully\n");

    // Clean up
    vv_dsp_fft_destroy(plan);
    return 0;
}
```

### Window Functions Example

```c
#include <vv_dsp/vv_dsp.h>

int main(void) {
    const size_t N = 512;
    vv_dsp_real window[N];

    // Create Hann window
    vv_dsp_status status = vv_dsp_window_hann(N, window);
    if (status != VV_DSP_OK) {
        fprintf(stderr, "Failed to create Hann window: %d\n", status);
        return 1;
    }

    // Apply window to signal
    vv_dsp_real signal[N];
    // ... fill signal with data ...

    for (size_t i = 0; i < N; i++) {
        signal[i] *= window[i];
    }

    printf("Window applied successfully\n");
    return 0;
}
```

### FIR Filter Example

```c
#include <vv_dsp/vv_dsp.h>

int main(void) {
    // Design lowpass filter
    const size_t filter_length = 64;
    const vv_dsp_real cutoff = 0.25f;  // Normalized frequency

    vv_dsp_real coeffs[filter_length];
    vv_dsp_status status = vv_dsp_fir_lowpass(filter_length, cutoff, coeffs);
    if (status != VV_DSP_OK) {
        fprintf(stderr, "Failed to design FIR filter: %d\n", status);
        return 1;
    }

    // Create filter context
    vv_dsp_fir_ctx* ctx;
    status = vv_dsp_fir_create(&ctx, coeffs, filter_length);
    if (status != VV_DSP_OK) {
        fprintf(stderr, "Failed to create FIR context: %d\n", status);
        return 1;
    }

    // Filter data
    const size_t data_length = 1024;
    vv_dsp_real input[data_length], output[data_length];

    // ... fill input with data ...

    status = vv_dsp_fir_process(ctx, input, output, data_length);
    if (status != VV_DSP_OK) {
        fprintf(stderr, "Failed to process FIR filter: %d\n", status);
        vv_dsp_fir_destroy(ctx);
        return 1;
    }

    printf("Filtering completed successfully\n");

    // Clean up
    vv_dsp_fir_destroy(ctx);
    return 0;
}
```

## Troubleshooting

### Common Issues

#### 1. CMake can't find vv-dsp

**Problem**: `find_package(vv-dsp CONFIG REQUIRED)` fails

**Solution**:
- For vcpkg: Ensure you're using the vcpkg toolchain file
- For FetchContent: Make sure the target name is correct (`vv-dsp` not `vv-dsp::vv-dsp`)
- For custom installation: Set `CMAKE_PREFIX_PATH` to the installation directory

#### 2. Linking errors

**Problem**: Undefined reference to vv-dsp functions

**Solutions**:
- Ensure you're linking against the correct target: `vv-dsp::vv-dsp` (vcpkg) or `vv-dsp` (FetchContent/submodule)
- On Unix systems, ensure the math library is linked: `target_link_libraries(my_app PRIVATE vv-dsp::vv-dsp m)`
- Check that required features are enabled if using advanced functions

#### 3. Header not found

**Problem**: `fatal error: 'vv_dsp/vv_dsp.h' file not found`

**Solutions**:
- Include the umbrella header: `#include <vv_dsp/vv_dsp.h>`
- Or include specific headers: `#include <vv_dsp/spectral/fft.h>`
- Ensure the target is properly linked

#### 4. Runtime errors

**Problem**: FFT functions return error codes

**Solutions**:
- Check that input sizes are valid (power of 2 for some backends)
- Ensure buffers are properly allocated
- Verify that required backends are available: `vv_dsp_fft_is_backend_available(backend)`

### Performance Tips

1. **FFT Backend Selection**: For best performance, use FFTW3 backend on desktop/server applications
2. **SIMD Optimizations**: Enable the `simd` feature for automatic vectorization
3. **Buffer Alignment**: Use aligned memory allocation for SIMD operations
4. **Plan Reuse**: Create FFT plans once and reuse them for multiple operations

### Platform-Specific Notes

#### Windows (MSVC)
- Math constants are automatically defined (`_USE_MATH_DEFINES`)
- CRT security warnings are disabled for library code
- Use `/W4` warning level for best results

#### macOS
- Xcode Command Line Tools required for compilation
- Homebrew vcpkg installation recommended: `brew install vcpkg`

#### Linux
- Install development packages: `sudo apt-get install build-essential cmake`
- For FFTW3: `sudo apt-get install libfftw3-dev`

## Support

For issues, questions, or contributions:
- GitHub Issues: https://github.com/crlotwhite/vv-dsp/issues
- Documentation: https://github.com/crlotwhite/vv-dsp/docs
- Examples: https://github.com/crlotwhite/vv-dsp/tree/main/examples

## License

vv-dsp is released under the MIT License. See the [LICENSE](../LICENSE) file for details.
