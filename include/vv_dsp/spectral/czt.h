#ifndef VV_DSP_SPECTRAL_CZT_H
#define VV_DSP_SPECTRAL_CZT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"

// Chirp Z-Transform (CZT)
// Evaluates X[k] = sum_{n=0}^{N-1} x[n] * A^{-n} * W^{n k},
// at points z_k = A * W^{-k} (SciPy-compatible convention).

// Helper to compute (W, A) for sampling a frequency arc on the unit circle
// from f_start (Hz) to f_end (Hz) with sampling_rate (Hz) using M points.
// z_k = exp(-j 2*pi*(f_start + k*delta)/fs),
// where delta = (f_end - f_start)/M.
vv_dsp_status vv_dsp_czt_params_for_freq_range(
    vv_dsp_real f_start,
    vv_dsp_real f_end,
    size_t M,
    vv_dsp_real sampling_rate,
    vv_dsp_real* W_real, vv_dsp_real* W_imag,
    vv_dsp_real* A_real, vv_dsp_real* A_imag);

// CZT for complex input
vv_dsp_status vv_dsp_czt_exec_cpx(
    const vv_dsp_cpx* input,
    size_t N,
    size_t M,
    vv_dsp_real W_real, vv_dsp_real W_imag,
    vv_dsp_real A_real, vv_dsp_real A_imag,
    vv_dsp_cpx* output);

// Convenience CZT for real input (imag=0). Output is complex.
vv_dsp_status vv_dsp_czt_exec_real(
    const vv_dsp_real* input,
    size_t N,
    size_t M,
    vv_dsp_real W_real, vv_dsp_real W_imag,
    vv_dsp_real A_real, vv_dsp_real A_imag,
    vv_dsp_cpx* output);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_SPECTRAL_CZT_H
