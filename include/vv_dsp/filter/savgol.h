/*
 * Savitzky–Golay filter API
 */
#ifndef VV_DSP_FILTER_SAVGOL_H
#define VV_DSP_FILTER_SAVGOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/**
 * Boundary handling modes for Savitzky–Golay filtering
 */
typedef enum vv_dsp_savgol_mode {
    VV_DSP_SAVGOL_MODE_REFLECT = 0,  /* symmetric reflection (…3,2,1,0,0,1,2,3,…) */
    VV_DSP_SAVGOL_MODE_CONSTANT = 1, /* pad with constant value equal to edge sample */
    VV_DSP_SAVGOL_MODE_NEAREST = 2,  /* repeat nearest edge sample */
    VV_DSP_SAVGOL_MODE_WRAP = 3      /* circular wrap-around */
} vv_dsp_savgol_mode;

/**
 * Apply Savitzky–Golay smoothing/differentiation.
 *
 * Arguments:
 *  - y: input signal of length N
 *  - N: number of samples
 *  - window_length: odd window length (must be > polyorder)
 *  - polyorder: polynomial order (>= 0)
 *  - deriv: derivative order (0 for smoothing), 0 <= deriv <= polyorder
 *  - delta: sample spacing (used for derivative scaling)
 *  - mode: boundary handling mode
 *  - output: output buffer of length N
 *
 * Returns vv_dsp_status code.
 */
vv_dsp_status vv_dsp_savgol(const vv_dsp_real* y,
                            size_t N,
                            int window_length,
                            int polyorder,
                            int deriv,
                            vv_dsp_real delta,
                            vv_dsp_savgol_mode mode,
                            vv_dsp_real* output);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_FILTER_SAVGOL_H
