#ifndef VV_DSP_WINDOW_H
#define VV_DSP_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

// Window functions API
// All functions fill `out[0..N-1]` with window coefficients.
// Returns VV_DSP_OK on success. Error codes:
//  - VV_DSP_ERROR_NULL_POINTER if out == NULL
//  - VV_DSP_ERROR_INVALID_SIZE if N == 0

// Rectangular (boxcar) window: w[n] = 1.0
vv_dsp_status vv_dsp_window_boxcar(size_t N, vv_dsp_real* out);

// Hann window (symmetric): w[n] = 0.5 - 0.5*cos(2*pi*n/(N-1))
vv_dsp_status vv_dsp_window_hann(size_t N, vv_dsp_real* out);

// Hamming window (symmetric): w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1))
vv_dsp_status vv_dsp_window_hamming(size_t N, vv_dsp_real* out);

// Blackman window (symmetric):
// w[n] = 0.42 - 0.5*cos(2*pi*n/(N-1)) + 0.08*cos(4*pi*n/(N-1))
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
