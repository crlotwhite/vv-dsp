// Window function implementations
#include "vv_dsp/window.h"

#include <math.h>
#include "vv_dsp/vv_dsp_math.h"
#include <stddef.h>


// Internal helper for common validation
static VV_DSP_INLINE vv_dsp_status vv_dsp__validate_window_args(size_t N, const vv_dsp_real* out) {
    if (!out) return VV_DSP_ERROR_NULL_POINTER;
    if (N == 0) return VV_DSP_ERROR_INVALID_SIZE;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_boxcar(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    for (size_t n = 0; n < N; ++n) {
        out[n] = (vv_dsp_real)1.0;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_hann(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
    vv_dsp_real c = (vv_dsp_real)VV_DSP_COS(two_pi_over * (vv_dsp_real)n);
        out[n] = (vv_dsp_real)0.5 - (vv_dsp_real)0.5 * c;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_hamming(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
    vv_dsp_real c = (vv_dsp_real)VV_DSP_COS(two_pi_over * (vv_dsp_real)n);
        out[n] = (vv_dsp_real)0.54 - (vv_dsp_real)0.46 * c;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_blackman(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = two_pi_over * (vv_dsp_real)n;
    vv_dsp_real c1 = (vv_dsp_real)VV_DSP_COS(x);
    vv_dsp_real c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0 * x);
        out[n] = (vv_dsp_real)0.42 - (vv_dsp_real)0.5 * c1 + (vv_dsp_real)0.08 * c2;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_blackman_harris(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real a0 = (vv_dsp_real)0.35875;
    const vv_dsp_real a1 = (vv_dsp_real)0.48829;
    const vv_dsp_real a2 = (vv_dsp_real)0.14128;
    const vv_dsp_real a3 = (vv_dsp_real)0.01168;
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = two_pi_over * (vv_dsp_real)n;
    vv_dsp_real c1 = (vv_dsp_real)VV_DSP_COS(x);
    vv_dsp_real c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0 * x);
    vv_dsp_real c3 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)3.0 * x);
        out[n] = a0 - a1 * c1 + a2 * c2 - a3 * c3;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_window_nuttall(size_t N, vv_dsp_real* out) {
    vv_dsp_status s = vv_dsp__validate_window_args(N, out);
    if (s != VV_DSP_OK) return s;
    if (N == 1) { out[0] = (vv_dsp_real)1.0; return VV_DSP_OK; }
    const vv_dsp_real a0 = (vv_dsp_real)0.3635819;
    const vv_dsp_real a1 = (vv_dsp_real)0.4891775;
    const vv_dsp_real a2 = (vv_dsp_real)0.1365995;
    const vv_dsp_real a3 = (vv_dsp_real)0.0106411;
    const vv_dsp_real denom = (vv_dsp_real)(N - 1);
    const vv_dsp_real two_pi_over = (vv_dsp_real)(VV_DSP_TWO_PI) / denom;
    for (size_t n = 0; n < N; ++n) {
        vv_dsp_real x = two_pi_over * (vv_dsp_real)n;
    vv_dsp_real c1 = (vv_dsp_real)VV_DSP_COS(x);
    vv_dsp_real c2 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)2.0 * x);
    vv_dsp_real c3 = (vv_dsp_real)VV_DSP_COS((vv_dsp_real)3.0 * x);
        out[n] = a0 - a1 * c1 + a2 * c2 - a3 * c3;
    }
    return VV_DSP_OK;
}
