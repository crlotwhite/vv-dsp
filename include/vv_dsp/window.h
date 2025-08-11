/**
 * @file window.h
 * @brief Window functions for signal processing
 * @ingroup window_group
 *
 * This module provides various window functions commonly used in signal processing
 * for spectral analysis, filtering, and other DSP applications. All windows are
 * computed with symmetric properties suitable for FFT analysis.
 */

#ifndef VV_DSP_WINDOW_H
#define VV_DSP_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/**
 * @defgroup window_group Window Functions
 * @brief Window functions for spectral analysis and filtering
 *
 * Window functions are used to reduce spectral leakage in FFT analysis and
 * to shape the impulse response of filters. This module provides commonly
 * used windows with symmetric properties.
 *
 * All window functions:
 * - Fill output array `out[0..N-1]` with window coefficients
 * - Return VV_DSP_OK on success
 * - Use symmetric formulation: w[n] = w[N-1-n]
 * - Have unit peak value (maximum = 1.0)
 *
 * @{
 */

/**
 * @brief Rectangular (boxcar) window
 * @param N Window length
 * @param out Output array for window coefficients
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Generates w[n] = 1.0 for all n. This is the simplest window
 * with no tapering, equivalent to no windowing at all.
 *
 * **Properties:**
 * - Best frequency resolution
 * - Highest spectral leakage
 * - Rectangular time domain shape
 */
vv_dsp_status vv_dsp_window_boxcar(size_t N, vv_dsp_real* out);

/**
 * @brief Hann window (raised cosine)
 * @param N Window length
 * @param out Output array for window coefficients
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Generates w[n] = 0.5 - 0.5*cos(2π*n/(N-1))
 *
 * **Properties:**
 * - Good compromise between resolution and leakage
 * - Smooth tapering to zero at edges
 * - Very commonly used for general-purpose analysis
 */
vv_dsp_status vv_dsp_window_hann(size_t N, vv_dsp_real* out);

/**
 * @brief Hamming window
 * @param N Window length
 * @param out Output array for window coefficients
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Generates w[n] = 0.54 - 0.46*cos(2π*n/(N-1))
 *
 * **Properties:**
 * - Better sidelobe suppression than Hann
 * - Non-zero values at edges (0.08)
 * - Slightly worse frequency resolution than Hann
 */
vv_dsp_status vv_dsp_window_hamming(size_t N, vv_dsp_real* out);

/**
 * @brief Blackman window
 * @param N Window length
 * @param out Output array for window coefficients
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Generates w[n] = 0.42 - 0.5*cos(2π*n/(N-1)) + 0.08*cos(4π*n/(N-1))
 *
 * **Properties:**
 * - Excellent sidelobe suppression
 * - Wider main lobe than Hann/Hamming
 * - Good for detecting weak signals near strong ones
 */
vv_dsp_status vv_dsp_window_blackman(size_t N, vv_dsp_real* out);

// 4-term Blackman-Harris (symmetric):
// a0=0.35875, a1=0.48829, a2=0.14128, a3=0.01168
// w[n] = a0 - a1*cos(2*pi*n/(N-1)) + a2*cos(4*pi*n/(N-1)) - a3*cos(6*pi*n/(N-1))
vv_dsp_status vv_dsp_window_blackman_harris(size_t N, vv_dsp_real* out);

// Nuttall (symmetric):
// a0=0.3635819, a1=0.4891775, a2=0.1365995, a3=0.0106411
// w[n] = a0 - a1*cos(2*pi*n/(N-1)) + a2*cos(4*pi*n/(N-1)) - a3*cos(6*pi*n/(N-1))
vv_dsp_status vv_dsp_window_nuttall(size_t N, vv_dsp_real* out);

// Bartlett (Triangular) window (symmetric):
// w[n] = 1 - |n - (N-1)/2| / ((N-1)/2)
vv_dsp_status vv_dsp_window_bartlett(size_t N, vv_dsp_real* out);

// Bohman window (symmetric):
// w[n] = (1-|x|)*cos(π*|x|) + sin(π*|x|)/π, where x = 2*n/(N-1) - 1
vv_dsp_status vv_dsp_window_bohman(size_t N, vv_dsp_real* out);

// Cosine window (symmetric):
// w[n] = sin(π*n/(N-1))
vv_dsp_status vv_dsp_window_cosine(size_t N, vv_dsp_real* out);

// Planck-taper window (symmetric):
// Exponential tapered window with default epsilon=0.1
vv_dsp_status vv_dsp_window_planck_taper(size_t N, vv_dsp_real* out);

// Flattop window (symmetric):
// 5-term cosine window optimized for amplitude accuracy
vv_dsp_status vv_dsp_window_flattop(size_t N, vv_dsp_real* out);

// Kaiser window (symmetric):
// w[n] = I_0(β*√(1-((n-(N-1)/2)/((N-1)/2))²)) / I_0(β)
// where I_0 is the modified Bessel function of the first kind, order 0
vv_dsp_status vv_dsp_window_kaiser(size_t N, vv_dsp_real beta, vv_dsp_real* out);

// Tukey window (symmetric):
// Tapered cosine window, alpha ∈ [0,1] controls taper fraction
// alpha=0: rectangular, alpha=1: Hann window
vv_dsp_status vv_dsp_window_tukey(size_t N, vv_dsp_real alpha, vv_dsp_real* out);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_WINDOW_H
