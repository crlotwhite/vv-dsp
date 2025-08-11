/**
 * @file vv_dsp_deps.h
 * @brief External dependency includes and utilities for vv-dsp
 *
 * This header provides optional integration with external libraries
 * to enhance vv-dsp performance and functionality.
 */

#ifndef VV_DSP_DEPS_H
#define VV_DSP_DEPS_H

#ifdef __cplusplus
extern "C" {
#endif

// FastApprox - Fast approximations to common mathematical functions
#ifdef VV_DSP_USE_FASTAPPROX
    #include "fastonebigheader.h"
    #define VV_DSP_FAST_EXP(x) fastexp(x)
    #define VV_DSP_FAST_LOG(x) fastlog(x)
    #define VV_DSP_FAST_POW(x, y) fastpow(x, y)
    #define VV_DSP_FAST_SIN(x) fastsin(x)
    #define VV_DSP_FAST_COS(x) fastcos(x)
#else
    #include <math.h>
    #define VV_DSP_FAST_EXP(x) exp(x)
    #define VV_DSP_FAST_LOG(x) log(x)
    #define VV_DSP_FAST_POW(x, y) pow(x, y)
    #define VV_DSP_FAST_SIN(x) sin(x)
    #define VV_DSP_FAST_COS(x) cos(x)
#endif

// Math Approx - DSP-optimized math approximations
#ifdef VV_DSP_USE_MATH_APPROX
    #ifdef __cplusplus
        #include "math_approx/math_approx.hpp"
        #define VV_DSP_APPROX_EXP(x) math_approx::exp_approx(x)
        #define VV_DSP_APPROX_LOG(x) math_approx::log_approx(x)
        #define VV_DSP_APPROX_SIN(x) math_approx::sin_approx(x)
        #define VV_DSP_APPROX_COS(x) math_approx::cos_approx(x)
        #define VV_DSP_APPROX_TAN(x) math_approx::tan_approx(x)
    #endif
#endif

// Eigen - Linear algebra library
#ifdef VV_DSP_USE_EIGEN
    #ifdef __cplusplus
        #include <Eigen/Dense>
        #include <Eigen/Core>
        // Type aliases for common Eigen types
        using VV_DSP_Matrix = Eigen::MatrixXd;
        using VV_DSP_Vector = Eigen::VectorXd;
        using VV_DSP_MatrixF = Eigen::MatrixXf;
        using VV_DSP_VectorF = Eigen::VectorXf;
    #endif
#endif

/**
 * @brief Check if fast approximation functions are available
 * @return 1 if fastapprox is available, 0 otherwise
 */
static inline int vv_dsp_has_fastapprox(void) {
#ifdef VV_DSP_USE_FASTAPPROX
    return 1;
#else
    return 0;
#endif
}

/**
 * @brief Check if DSP-optimized math approximations are available
 * @return 1 if math_approx is available, 0 otherwise
 */
static inline int vv_dsp_has_math_approx(void) {
#ifdef VV_DSP_USE_MATH_APPROX
    return 1;
#else
    return 0;
#endif
}

/**
 * @brief Check if Eigen linear algebra library is available
 * @return 1 if Eigen is available, 0 otherwise
 */
static inline int vv_dsp_has_eigen(void) {
#ifdef VV_DSP_USE_EIGEN
    return 1;
#else
    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* VV_DSP_DEPS_H */
