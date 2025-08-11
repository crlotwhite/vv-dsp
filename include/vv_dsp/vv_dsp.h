/**
 * @file vv_dsp.h
 * @brief VV-DSP: A Modular, Portable, and Testable Digital Signal Processing Library
 *
 * @mainpage VV-DSP Library Documentation
 *
 * @section intro_sec Introduction
 *
 * VV-DSP is a comprehensive C99-based digital signal processing library designed with
 * modularity, portability, and testability in mind. The library provides essential
 * DSP operations including FFT, filtering, resampling, spectral analysis, and more.
 *
 * @section features_sec Key Features
 *
 * - **Modular Architecture**: Clean separation of concerns across multiple modules
 * - **Portable**: Supports multiple platforms (Linux, Windows MSVC, macOS)
 * - **Testable**: Comprehensive test suite with Python cross-validation
 * - **Configurable**: CMake-based build system with extensive options
 * - **Performance**: SIMD optimizations and pluggable FFT backends
 * - **Standards Compliant**: C99 standard with optional C++20 features
 *
 * @section modules_sec Library Modules
 *
 * - @ref core_group "Core": Basic mathematical operations and statistics
 * - @ref spectral_group "Spectral": FFT, STFT, DCT, CZT, and Hilbert transforms
 * - @ref filter_group "Filter": FIR, IIR, and Savitzky-Golay filters
 * - @ref resample_group "Resample": Sample rate conversion and interpolation
 * - @ref envelope_group "Envelope": Signal envelope extraction
 * - @ref window_group "Window": Various windowing functions
 * - @ref features_group "Features": MFCC and other feature extraction
 * - @ref audio_group "Audio": Audio I/O operations (optional)
 * - @ref adapters_group "Adapters": External library integration
 *
 * @section usage_sec Basic Usage
 *
 * @code{.c}
 * #include "vv_dsp/vv_dsp.h"
 *
 * int main() {
 *     // Example: Compute FFT of a signal
 *     vv_dsp_cpx input[256];
 *     vv_dsp_cpx output[256];
 *
 *     // Initialize input signal
 *     for (int i = 0; i < 256; i++) {
 *         input[i] = vv_dsp_cpx_make(sin(2*M_PI*i/256), 0.0);
 *     }
 *
 *     // Perform FFT
 *     vv_dsp_fft_c2c(input, output, 256, VV_DSP_FFT_FORWARD);
 *
 *     return 0;
 * }
 * @endcode
 *
 * @section build_sec Building the Library
 *
 * VV-DSP uses CMake for building:
 *
 * @code{.bash}
 * mkdir build && cd build
 * cmake -DCMAKE_BUILD_TYPE=Release ..
 * cmake --build .
 * ctest  # Run tests
 * @endcode
 *
 * @section license_sec License
 *
 * This project is licensed under the MIT License.
 *
 * @version 0.1.0
 * @author VV-DSP Development Team
 * @date 2025
 */

#ifndef VV_DSP_H
#define VV_DSP_H

#ifdef __cplusplus
extern "C" {
#endif
// VV-DSP umbrella header - includes all public module headers
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/vv_dsp_deps.h"
#include "vv_dsp/core.h"
#include "vv_dsp/spectral.h"
#include "vv_dsp/spectral/dct.h"
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/filter.h"
#include "vv_dsp/resample.h"
#include "vv_dsp/envelope.h"
#include "vv_dsp/window.h"
#include "vv_dsp/adapters.h"
#include "vv_dsp/features/mel.h"

#ifdef VV_DSP_AUDIO_ENABLED
#include "vv_dsp/audio.h"
#endif

/** @name Library Version Information
 * @{
 */
/** Major version number */
#define VV_DSP_VERSION_MAJOR 0
/** Minor version number */
#define VV_DSP_VERSION_MINOR 1
/** Patch version number */
#define VV_DSP_VERSION_PATCH 0

/**
 * @brief Create version string for display
 * @details Constructs a version string in the format "MAJOR.MINOR.PATCH"
 */
#define VV_DSP_VERSION_STRING "0.1.0"
/** @} */

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_H
