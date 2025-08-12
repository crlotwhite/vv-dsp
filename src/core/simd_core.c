/**
 * @file simd_core.c
 * @brief SIMD optimized DSP functions implementation
 */

#include "../../include/vv_dsp/core/simd_core.h"
#include "../../include/vv_dsp/core/simd_utils.h"
#include <math.h>

// Fallback implementations (always available)
vv_dsp_status vv_dsp_add_real_simd(const vv_dsp_real* a, const vv_dsp_real* b,
                                   vv_dsp_real* out, size_t n) {
    if (!a || !b || !out) return VV_DSP_ERROR_NULL_POINTER;

    for (size_t i = 0; i < n; i++) {
        out[i] = a[i] + b[i];
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_mul_real_simd(const vv_dsp_real* a, const vv_dsp_real* b,
                                   vv_dsp_real* out, size_t n) {
    if (!a || !b || !out) return VV_DSP_ERROR_NULL_POINTER;

    for (size_t i = 0; i < n; i++) {
        out[i] = a[i] * b[i];
    }
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_sum_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;

    vv_dsp_real sum = 0.0f;
    for (size_t i = 0; i < n; i++) {
        sum += x[i];
    }
    *out = sum;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_rms_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n == 0) {
        *out = 0.0f;
        return VV_DSP_OK;
    }

    vv_dsp_real sum_sq = 0.0f;
    for (size_t i = 0; i < n; i++) {
        sum_sq += x[i] * x[i];
    }
    *out = sqrtf(sum_sq / (vv_dsp_real)n);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_peak_optimized(const vv_dsp_real* x, size_t n,
                                    vv_dsp_real* min_val, vv_dsp_real* max_val) {
    if (!x || n == 0) return VV_DSP_ERROR_NULL_POINTER;

    vv_dsp_real min_result = x[0];
    vv_dsp_real max_result = x[0];

    for (size_t i = 1; i < n; i++) {
        if (x[i] < min_result) min_result = x[i];
        if (x[i] > max_result) max_result = x[i];
    }

    if (min_val) *min_val = min_result;
    if (max_val) *max_val = max_result;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_mean_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;

    vv_dsp_real sum;
    vv_dsp_status status = vv_dsp_sum_optimized(x, n, &sum);
    if (status != VV_DSP_OK) return status;

    *out = sum / (vv_dsp_real)n;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_variance_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n <= 1) return VV_DSP_ERROR_INVALID_SIZE;

    vv_dsp_real mean;
    vv_dsp_status status = vv_dsp_mean_optimized(x, n, &mean);
    if (status != VV_DSP_OK) return status;

    vv_dsp_real sum_sq_dev = 0.0f;
    for (size_t i = 0; i < n; i++) {
        vv_dsp_real diff = x[i] - mean;
        sum_sq_dev += diff * diff;
    }

    *out = sum_sq_dev / (vv_dsp_real)(n - 1);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_population_variance_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;
    if (n == 0) return VV_DSP_ERROR_INVALID_SIZE;

    vv_dsp_real mean;
    vv_dsp_status status = vv_dsp_mean_optimized(x, n, &mean);
    if (status != VV_DSP_OK) return status;

    vv_dsp_real sum_sq_dev = 0.0f;
    for (size_t i = 0; i < n; i++) {
        vv_dsp_real diff = x[i] - mean;
        sum_sq_dev += diff * diff;
    }

    *out = sum_sq_dev / (vv_dsp_real)n;
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_stddev_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;

    vv_dsp_real variance;
    vv_dsp_status status = vv_dsp_variance_optimized(x, n, &variance);
    if (status != VV_DSP_OK) return status;

    *out = sqrtf(variance);
    return VV_DSP_OK;
}

vv_dsp_status vv_dsp_population_stddev_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out) {
    if (!x || !out) return VV_DSP_ERROR_NULL_POINTER;

    vv_dsp_real variance;
    vv_dsp_status status = vv_dsp_population_variance_optimized(x, n, &variance);
    if (status != VV_DSP_OK) return status;

    *out = sqrtf(variance);
    return VV_DSP_OK;
}
