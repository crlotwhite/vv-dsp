#ifndef VV_DSP_ENVELOPE_LPC_H
#define VV_DSP_ENVELOPE_LPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"

// Autocorrelation of real signal
vv_dsp_status vv_dsp_autocorr(const vv_dsp_real* x, size_t n, size_t order, vv_dsp_real* r_out);

// Levinson-Durbin to solve Toeplitz system for LPC coefficients
vv_dsp_status vv_dsp_levinson(const vv_dsp_real* r, size_t order, vv_dsp_real* a_out, vv_dsp_real* err_out);

// Compute LPC coefficients and return prediction error
vv_dsp_status vv_dsp_lpc(const vv_dsp_real* x, size_t n, size_t order, vv_dsp_real* a_out, vv_dsp_real* err_out);

// From LPC coefficients compute magnitude spectrum envelope (simple dense sampling)
vv_dsp_status vv_dsp_lpspec(const vv_dsp_real* a, size_t order, vv_dsp_real gain, size_t nfft, vv_dsp_real* mag_out);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_ENVELOPE_LPC_H
