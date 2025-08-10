/*
 * FIR filter API
 */
#ifndef VV_DSP_FILTER_FIR_H
#define VV_DSP_FILTER_FIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/filter/common.h"

/**
 * Design a low-pass FIR using windowed-sinc.
 * @param coeffs Output coefficients array of length num_taps
 * @param num_taps Number of taps (filter order + 1)
 * @param cutoff_norm Normalized cutoff 0..1 (1 = Nyquist)
 * @param window_type Window function selection
 * @return VV_DSP_OK on success
 */
vv_dsp_status vv_dsp_fir_design_lowpass(vv_dsp_real* coeffs,
                                        size_t num_taps,
                                        vv_dsp_real cutoff_norm, // 0..1 (1=Nyquist)
                                        vv_dsp_window_type window_type);

// FIR streaming state
typedef struct {
    vv_dsp_real* history;    // ring buffer of previous samples, size=num_taps-1
    size_t history_size;     // equals num_taps-1 when initialized
    size_t history_idx;      // write index
    size_t num_taps;         // number of coefficients
} vv_dsp_fir_state;

vv_dsp_status vv_dsp_fir_state_init(vv_dsp_fir_state* state, size_t num_taps);
void vv_dsp_fir_state_free(vv_dsp_fir_state* state);

// Apply FIR via direct convolution with state (overlap via history)
vv_dsp_status vv_dsp_fir_apply(vv_dsp_fir_state* state,
                               const vv_dsp_real* coeffs,
                               const vv_dsp_real* input,
                               vv_dsp_real* output,
                               size_t num_samples);

/**
 * Apply FIR via FFT-based convolution (single block, linear convolution).
 * Produces output length equal to num_samples (tail is truncated like time-domain apply with zero history).
 */
vv_dsp_status vv_dsp_fir_apply_fft(vv_dsp_fir_state* state,
                                   const vv_dsp_real* coeffs,
                                   const vv_dsp_real* input,
                                   vv_dsp_real* output,
                                   size_t num_samples);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_FILTER_FIR_H
