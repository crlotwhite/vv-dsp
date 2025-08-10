/*
 * Common filter utilities
 */
#ifndef VV_DSP_FILTER_COMMON_H
#define VV_DSP_FILTER_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/** Window type enum used by FIR design */
typedef enum {
    VV_DSP_WINDOW_RECTANGULAR = 0,
    VV_DSP_WINDOW_HAMMING = 1,
    VV_DSP_WINDOW_HANNING = 2,
    VV_DSP_WINDOW_BLACKMAN = 3
} vv_dsp_window_type;

/**
 * Zero-phase filtering for FIR by forward-backward pass with reflection padding.
 * Applies the FIR filter forward, reverses, applies again, then reverses back.
 * A simple symmetric reflection padding (num_taps-1) minimizes edge transients.
 */
vv_dsp_status vv_dsp_filtfilt_fir(const vv_dsp_real* coeffs,
                                  size_t num_taps,
                                  const vv_dsp_real* input,
                                  vv_dsp_real* output,
                                  size_t num_samples);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_FILTER_COMMON_H
