#ifndef VV_DSP_CORE_H
#define VV_DSP_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

// Core DSP utilities API

// Simple add function for test
int vv_dsp_add_int(int a, int b);

// Complex helpers
vv_dsp_cpx vv_dsp_cpx_add(vv_dsp_cpx a, vv_dsp_cpx b);
vv_dsp_cpx vv_dsp_cpx_mul(vv_dsp_cpx a, vv_dsp_cpx b);
vv_dsp_cpx vv_dsp_cpx_conj(vv_dsp_cpx z);
vv_dsp_real vv_dsp_cpx_abs(vv_dsp_cpx z);
vv_dsp_real vv_dsp_cpx_phase(vv_dsp_cpx z);
vv_dsp_cpx vv_dsp_cpx_from_polar(vv_dsp_real r, vv_dsp_real theta);

// Basic math on arrays (real)
vv_dsp_status vv_dsp_sum(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_mean(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_var(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_min(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_max(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

// Index of min/max (argmin/argmax) returns via pointer
vv_dsp_status vv_dsp_argmin(const vv_dsp_real* x, size_t n, size_t* idx);
vv_dsp_status vv_dsp_argmax(const vv_dsp_real* x, size_t n, size_t* idx);

// Utilities
vv_dsp_status vv_dsp_cumsum(const vv_dsp_real* x, size_t n, vv_dsp_real* y);
vv_dsp_status vv_dsp_diff(const vv_dsp_real* x, size_t n, vv_dsp_real* y); // y size n-1
vv_dsp_real vv_dsp_clamp(vv_dsp_real v, vv_dsp_real lo, vv_dsp_real hi);
void vv_dsp_flush_denormals(void);

// -------- Statistics & measurement utilities --------
// All functions operate on vv_dsp_real (float by default, double if VV_DSP_USE_DOUBLE defined)
vv_dsp_status vv_dsp_rms(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_peak(const vv_dsp_real* x, size_t n, vv_dsp_real* min_val, vv_dsp_real* max_val);
vv_dsp_status vv_dsp_crest_factor(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_zero_crossing_rate(const vv_dsp_real* x, size_t n, size_t* count_out);
vv_dsp_status vv_dsp_skewness(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_kurtosis(const vv_dsp_real* x, size_t n, vv_dsp_real* out);
vv_dsp_status vv_dsp_autocorrelation(const vv_dsp_real* x, size_t n, vv_dsp_real* r, size_t r_len, int biased);
vv_dsp_status vv_dsp_cross_correlation(const vv_dsp_real* x, size_t nx,
									   const vv_dsp_real* y, size_t ny,
									   vv_dsp_real* r, size_t r_len);

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_CORE_H
