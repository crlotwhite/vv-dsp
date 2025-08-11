#include "vv_dsp/core.h"
#include <math.h>
#include <float.h>
#include <stddef.h>

static VV_DSP_INLINE int invalid_in(const vv_dsp_real* x, size_t n) {
    return (x == NULL || n == 0);
}

vv_dsp_status vv_dsp_rms(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || invalid_in(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    double acc = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double v = (double)x[i];
        acc += v * v;
    }
    *out = (vv_dsp_real)sqrt(acc / (double)n);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_peak(const vv_dsp_real* x, size_t n, vv_dsp_real* min_val, vv_dsp_real* max_val) {
    if (invalid_in(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_real mn = x[0];
    vv_dsp_real mx = x[0];
    for (size_t i = 1; i < n; ++i) {
        if (x[i] < mn) mn = x[i];
        if (x[i] > mx) mx = x[i];
    }
    if (min_val) *min_val = mn;
    if (max_val) *max_val = mx;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_crest_factor(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || invalid_in(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    vv_dsp_real mn, mx;
    vv_dsp_status st = vv_dsp_peak(x, n, &mn, &mx);
    if (st != VV_DSP_OK) return st;
    vv_dsp_real peak = (mx > -mn) ? mx : -mn; // max absolute
    vv_dsp_real r;
    st = vv_dsp_rms(x, n, &r);
    if (st != VV_DSP_OK) return st;
    if (r == (vv_dsp_real)0) { *out = INFINITY; return VV_DSP_OK; }
    *out = peak / r;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_zero_crossing_rate(const vv_dsp_real* x, size_t n, size_t* count_out) {
    if (!count_out || invalid_in(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    size_t c = 0;
    for (size_t i = 1; i < n; ++i) {
        vv_dsp_real a = x[i-1];
        vv_dsp_real b = x[i];
        if ((a > 0 && b < 0) || (a < 0 && b > 0)) c++;
    }
    *count_out = c;
    return VV_DSP_OK;
}

// One-pass computation for mean, variance, skewness (m3), kurtosis (m4)
vv_dsp_status vv_dsp_skewness(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || invalid_in(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    if (n < 3) { *out = 0; return VV_DSP_ERROR_INVALID_SIZE; }
    double mean = 0.0, m2 = 0.0, m3 = 0.0; size_t k = 0;
    for (size_t i = 0; i < n; ++i) {
        double xi = (double)x[i];
        size_t k1 = ++k;
        double delta = xi - mean;
        double delta_n = delta / (double)k1;
        double term1 = delta * delta_n * (double)(k - 1);
        m3 += term1 * delta_n * (double)(k - 2) - 3.0 * delta_n * m2;
        m2 += term1;
        mean += delta_n;
    }
    double var = m2 / (double)n;
    if (var <= 0.0) { *out = 0; return VV_DSP_OK; }
    double s = m3 / (double)n;
    *out = (vv_dsp_real)(s / pow(var, 1.5));
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_kurtosis(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!out || invalid_in(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    if (n < 4) { *out = 0; return VV_DSP_ERROR_INVALID_SIZE; }
    double mean = 0.0, m2 = 0.0, m3 = 0.0, m4 = 0.0; size_t k = 0;
    for (size_t i = 0; i < n; ++i) {
        double xi = (double)x[i];
        size_t k1 = ++k;
        double delta = xi - mean;
        double delta_n = delta / (double)k1;
        double delta_n2 = delta_n * delta_n;
        double term1 = delta * delta_n * (double)(k - 1);
        m4 += term1 * delta_n2 * ((double)k * (double)k - 3.0*(double)k + 3.0)
              + 6.0 * delta_n2 * m2 - 4.0 * delta_n * m3;
        m3 += term1 * delta_n * (double)(k - 2) - 3.0 * delta_n * m2;
        m2 += term1;
        mean += delta_n;
    }
    double var = m2 / (double)n;
    if (var <= 0.0) { *out = 0; return VV_DSP_OK; }
    double k_excess = (m4 / (double)n) / (var * var) - 3.0; // excess kurtosis
    *out = (vv_dsp_real)k_excess;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_autocorrelation(const vv_dsp_real* x, size_t n, vv_dsp_real* r, size_t r_len, int biased) {
    if (!r || invalid_in(x, n)) return VV_DSP_ERROR_NULL_POINTER;
    if (r_len == 0) return VV_DSP_ERROR_INVALID_SIZE;
    size_t maxlag = r_len - 1;
    for (size_t lag = 0; lag <= maxlag; ++lag) {
        double acc = 0.0;
        size_t count = 0;
        for (size_t i = 0; i + lag < n; ++i) {
            acc += (double)x[i] * (double)x[i + lag];
            count++;
        }
        if (count == 0) { r[lag] = 0; continue; }
        if (biased) r[lag] = (vv_dsp_real)(acc / (double)n);
        else r[lag] = (vv_dsp_real)(acc / (double)count);
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_cross_correlation(const vv_dsp_real* x, size_t nx,
                                       const vv_dsp_real* y, size_t ny,
                                       vv_dsp_real* r, size_t r_len) {
    if (!r || invalid_in(x, nx) || invalid_in(y, ny)) return VV_DSP_ERROR_NULL_POINTER;
    if (r_len == 0) return VV_DSP_ERROR_INVALID_SIZE;
    // Define r[k] for k = 0..r_len-1 as correlation with lag k (y delayed by k)
    for (size_t lag = 0; lag < r_len; ++lag) {
        double acc = 0.0; size_t count = 0;
        for (size_t i = 0; i < nx; ++i) {
            size_t j = i + lag;
            if (j < ny) { acc += (double)x[i] * (double)y[j]; count++; }
        }
        r[lag] = (count > 0) ? (vv_dsp_real)(acc / (double)count) : (vv_dsp_real)0;
    }
    return VV_DSP_OK;
}
