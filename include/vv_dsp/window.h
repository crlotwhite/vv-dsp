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

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_WINDOW_H
