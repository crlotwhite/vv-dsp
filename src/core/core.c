#include "vv_dsp/core.h"
#include <math.h>
#include <float.h>

int vv_dsp_add_int(int a, int b) {
    return a + b;
}

// ---------- Complex helpers ----------
vv_dsp_cpx vv_dsp_cpx_add(vv_dsp_cpx a, vv_dsp_cpx b) {
    vv_dsp_cpx r; r.re = a.re + b.re; r.im = a.im + b.im; return r;
}

vv_dsp_cpx vv_dsp_cpx_mul(vv_dsp_cpx a, vv_dsp_cpx b) {
    vv_dsp_cpx r; r.re = a.re*b.re - a.im*b.im; r.im = a.re*b.im + a.im*b.re; return r;
}

vv_dsp_cpx vv_dsp_cpx_conj(vv_dsp_cpx z) {
    vv_dsp_cpx r; r.re = z.re; r.im = -z.im; return r;
}

vv_dsp_real vv_dsp_cpx_abs(vv_dsp_cpx z) {
    // hypot is stable for large/small values
    return (vv_dsp_real)hypot((double)z.re, (double)z.im);
}

vv_dsp_real vv_dsp_cpx_phase(vv_dsp_cpx z) {
    return (vv_dsp_real)atan2((double)z.im, (double)z.re);
}

vv_dsp_cpx vv_dsp_cpx_from_polar(vv_dsp_real r, vv_dsp_real theta) {
    double st = sin((double)theta);
    double ct = cos((double)theta);
    vv_dsp_cpx z; z.re = (vv_dsp_real)(ct * (double)r); z.im = (vv_dsp_real)(st * (double)r); return z;
}

// ---------- Basic math (real) ----------
static VV_DSP_INLINE int vv_dsp_is_invalid_input(const vv_dsp_real* x, size_t n) {
    return (x == NULL || n == 0);
}

vv_dsp_status vv_dsp_sum(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    // Kahan summation for better accuracy
    double sum = 0.0, c = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double y = (double)x[i] - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    *out = (vv_dsp_real)sum;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_mean(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_real s = 0;
    vv_dsp_status st = vv_dsp_sum(x, n, &s);
    if (st != VV_DSP_OK) return st;
    *out = s / (vv_dsp_real)n;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_var(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    if (n < 2) { *out = 0; return VV_DSP_ERROR_INVALID_SIZE; }
    // Welford's algorithm
    double mean = 0.0; double m2 = 0.0; size_t k = 0;
    for (size_t i = 0; i < n; ++i) {
        k++;
        double xk = (double)x[i];
        double delta = xk - mean;
        mean += delta / (double)k;
        double delta2 = xk - mean;
        m2 += delta * delta2;
    }
    *out = (vv_dsp_real)(m2 / (double)n);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_min(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_real m = x[0];
    for (size_t i = 1; i < n; ++i) if (x[i] < m) m = x[i];
    *out = m; return VV_DSP_OK;
}

vv_dsp_status vv_dsp_max(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_real m = x[0];
    for (size_t i = 1; i < n; ++i) if (x[i] > m) m = x[i];
    *out = m; return VV_DSP_OK;
}

vv_dsp_status vv_dsp_argmin(const vv_dsp_real* x, size_t n, size_t* idx) {
    if (!idx || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    size_t mi = 0; vv_dsp_real mv = x[0];
    for (size_t i = 1; i < n; ++i) if (x[i] < mv) { mv = x[i]; mi = i; }
    *idx = mi; return VV_DSP_OK;
}

vv_dsp_status vv_dsp_argmax(const vv_dsp_real* x, size_t n, size_t* idx) {
    if (!idx || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    size_t mi = 0; vv_dsp_real mv = x[0];
    for (size_t i = 1; i < n; ++i) if (x[i] > mv) { mv = x[i]; mi = i; }
    *idx = mi; return VV_DSP_OK;
}

// ---------- Utilities ----------
vv_dsp_status vv_dsp_cumsum(const vv_dsp_real* x, size_t n, vv_dsp_real* y) {
    if (!y || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_real acc = 0;
    for (size_t i = 0; i < n; ++i) {
        acc += x[i];
        y[i] = acc;
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_diff(const vv_dsp_real* x, size_t n, vv_dsp_real* y) {
    if (!y || vv_dsp_is_invalid_input(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    if (n < 2) return VV_DSP_ERROR_INVALID_SIZE;
    for (size_t i = 1; i < n; ++i) y[i-1] = x[i] - x[i-1];
    return VV_DSP_OK;
}

vv_dsp_real vv_dsp_clamp(vv_dsp_real v, vv_dsp_real lo, vv_dsp_real hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void vv_dsp_flush_denormals(void) {
    // Best-effort: set FTZ/DAZ on supported platforms (no-op portable fallback)
    // On x86 with SSE: could set MXCSR bits 15 (FTZ) and 6 (DAZ). Omitted for portability.
    (void)0;
}
