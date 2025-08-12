/**
 * @file vv_dsp_vectorized_math.h
 * @brief Vectorized math operations using Eigen library
 * @ingroup core_group
 *
 * This module provides high-performance vectorized implementations of common
 * DSP math operations using the Eigen library when available. These functions
 * are designed to replace element-wise loops with SIMD-optimized operations.
 */

#ifndef VV_DSP_VECTORIZED_MATH_H
#define VV_DSP_VECTORIZED_MATH_H

#include "vv_dsp/vv_dsp_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup core_group
 * @{
 */

/**
 * @brief Apply window function to a buffer using vectorized operations
 *
 * @param in Input signal buffer
 * @param window Window function coefficients
 * @param out Output buffer (can be same as in for in-place operation)
 * @param n Length of buffers
 * @return VV_DSP_OK on success, error code otherwise
 *
 * @details This function performs element-wise multiplication of input signal
 * with window coefficients using vectorized operations when Eigen is available,
 * falling back to scalar implementation otherwise.
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_vectorized_window_apply(
    const vv_dsp_real* in,
    const vv_dsp_real* window,
    vv_dsp_real* out,
    size_t n
);

/**
 * @brief Apply complex pointwise multiplication using vectorized operations
 *
 * @param a First complex array
 * @param b Second complex array
 * @param result Output array (can be same as a or b for in-place)
 * @param n Number of complex samples
 * @return VV_DSP_OK on success, error code otherwise
 *
 * @details Performs result[i] = a[i] * b[i] for complex numbers using
 * vectorized operations when available.
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_vectorized_complex_multiply(
    const vv_dsp_cpx* a,
    const vv_dsp_cpx* b,
    vv_dsp_cpx* result,
    size_t n
);

/**
 * @brief Apply vectorized trigonometric functions to an array
 *
 * @param in Input array
 * @param out Output array (can be same as in)
 * @param n Array length
 * @param func_type Function type: 0=sin, 1=cos, 2=tan
 * @return VV_DSP_OK on success, error code otherwise
 *
 * @details Applies trigonometric functions element-wise using vectorized
 * operations when available, with math approximation support.
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_vectorized_trig_apply(
    const vv_dsp_real* in,
    vv_dsp_real* out,
    size_t n,
    int func_type
);

/**
 * @brief Check if vectorized math operations are available
 *
 * @return 1 if Eigen vectorization is available, 0 otherwise
 */
int vv_dsp_vectorized_math_available(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* VV_DSP_VECTORIZED_MATH_H */
