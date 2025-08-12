/**
 * @file simd_core.h
 * @brief SIMD optimized DSP function declarations
 * @ingroup core
 */

#ifndef VV_DSP_SIMD_CORE_H
#define VV_DSP_SIMD_CORE_H

#include "vv_dsp/vv_dsp_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Element-wise addition of two arrays (SIMD optimized)
 * @param a First input array
 * @param b Second input array
 * @param out Output array (can be same as a or b for in-place operation)
 * @param n Number of elements
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_add_real_simd(const vv_dsp_real* a, const vv_dsp_real* b,
                                   vv_dsp_real* out, size_t n);

/**
 * @brief Element-wise multiplication of two arrays (SIMD optimized)
 * @param a First input array
 * @param b Second input array
 * @param out Output array (can be same as a or b for in-place operation)
 * @param n Number of elements
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_mul_real_simd(const vv_dsp_real* a, const vv_dsp_real* b,
                                   vv_dsp_real* out, size_t n);

/**
 * @brief SIMD-optimized sum function
 * @param x Input array
 * @param n Number of elements
 * @param out Output sum
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_sum_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief SIMD-optimized RMS function
 * @param x Input array
 * @param n Number of elements
 * @param out Output RMS value
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_rms_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief SIMD-optimized peak finding (min/max)
 * @param x Input array
 * @param n Number of elements
 * @param min_val Output minimum value (can be NULL)
 * @param max_val Output maximum value (can be NULL)
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_peak_optimized(const vv_dsp_real* x, size_t n,
                                    vv_dsp_real* min_val, vv_dsp_real* max_val);

/**
 * @brief SIMD-optimized mean function
 * @param x Input array
 * @param n Number of elements
 * @param out Output mean value
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_mean_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief SIMD-optimized variance function (unbiased estimator)
 * @param x Input array
 * @param n Number of elements
 * @param out Output variance value
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_variance_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief SIMD-optimized population variance function (biased estimator)
 * @param x Input array
 * @param n Number of elements
 * @param out Output population variance value
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_population_variance_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief SIMD-optimized standard deviation function (unbiased estimator)
 * @param x Input array
 * @param n Number of elements
 * @param out Output standard deviation value
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_stddev_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief SIMD-optimized population standard deviation function (biased estimator)
 * @param x Input array
 * @param n Number of elements
 * @param out Output population standard deviation value
 * @return VV_DSP_OK on success, error code on failure
 */
vv_dsp_status vv_dsp_population_stddev_optimized(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

#ifdef __cplusplus
}
#endif

#endif /* VV_DSP_SIMD_CORE_H */
