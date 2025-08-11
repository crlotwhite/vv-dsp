#ifndef VV_DSP_SPECTRAL_HILBERT_H
#define VV_DSP_SPECTRAL_HILBERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"

// Compute analytic signal of a real-valued input using FFT-based Hilbert transform.
// input: real[N]
// analytic_output: complex[N]
// Returns VV_DSP_OK on success; error code otherwise.
vv_dsp_status vv_dsp_hilbert_analytic(const vv_dsp_real* input, size_t N, vv_dsp_cpx* analytic_output);

// Compute instantaneous phase (radians) from analytic signal and unwrap it.
// analytic_input: complex[N]
// phase_output: real[N] (unwrapped phase)
vv_dsp_status vv_dsp_instantaneous_phase(const vv_dsp_cpx* analytic_input, size_t N, vv_dsp_real* phase_output);

// Compute instantaneous frequency (Hz) from unwrapped phase.
// unwrapped_phase_input: real[N]
// freq_output: real[N] (freq_output[0] = 0)
vv_dsp_status vv_dsp_instantaneous_frequency(const vv_dsp_real* unwrapped_phase_input,
                                             size_t N,
                                             double sample_rate,
                                             vv_dsp_real* freq_output);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_SPECTRAL_HILBERT_H
