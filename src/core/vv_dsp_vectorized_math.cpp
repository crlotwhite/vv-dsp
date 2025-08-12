/**
 * @file vv_dsp_vectorized_math.cpp
 * @brief Vectorized math operations implementation using Eigen
 */

#include "vv_dsp/core/vv_dsp_vectorized_math.h"
#include "vv_dsp/vv_dsp_math.h"

#ifdef VV_DSP_USE_EIGEN
#include <Eigen/Dense>
#include <Eigen/Core>
#include <cmath>
#endif

extern "C" {

int vv_dsp_vectorized_math_available(void) {
#ifdef VV_DSP_USE_EIGEN
    return 1;
#else
    return 0;
#endif
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_vectorized_window_apply(
    const vv_dsp_real* in,
    const vv_dsp_real* window,
    vv_dsp_real* out,
    size_t n
) {
    if (!in || !window || !out || n == 0) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

#ifdef VV_DSP_USE_EIGEN
    // Use Eigen for vectorized operations
    using VectorType = Eigen::Map<const Eigen::VectorXf>;
    using MutableVectorType = Eigen::Map<Eigen::VectorXf>;

    try {
        // Map input arrays to Eigen vectors
        VectorType input_vec(reinterpret_cast<const float*>(in), static_cast<Eigen::Index>(n));
        VectorType window_vec(reinterpret_cast<const float*>(window), static_cast<Eigen::Index>(n));
        MutableVectorType output_vec(reinterpret_cast<float*>(out), static_cast<Eigen::Index>(n));

        // Vectorized element-wise multiplication
        output_vec = input_vec.cwiseProduct(window_vec);

        return VV_DSP_OK;
    } catch (...) {
        // Fall back to scalar implementation on any Eigen error
    }
#endif

    // Scalar fallback implementation
    for (size_t i = 0; i < n; ++i) {
        out[i] = in[i] * window[i];
    }

    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_vectorized_complex_multiply(
    const vv_dsp_cpx* a,
    const vv_dsp_cpx* b,
    vv_dsp_cpx* result,
    size_t n
) {
    if (!a || !b || !result || n == 0) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

#ifdef VV_DSP_USE_EIGEN
    try {
        // Eigen doesn't have direct complex array operations in the same way,
        // so we'll treat complex numbers as 2-component vectors
        using VectorType = Eigen::Map<const Eigen::VectorXf>;
        using MutableVectorType = Eigen::Map<Eigen::VectorXf>;

        const size_t vec_size = n * 2; // Real and imaginary parts

        // Map arrays treating complex as [re, im, re, im, ...]
        VectorType a_vec(reinterpret_cast<const float*>(a), static_cast<Eigen::Index>(vec_size));
        VectorType b_vec(reinterpret_cast<const float*>(b), static_cast<Eigen::Index>(vec_size));
        MutableVectorType result_vec(reinterpret_cast<float*>(result), static_cast<Eigen::Index>(vec_size));

        // Complex multiplication: (a + bi)(c + di) = (ac - bd) + (ad + bc)i
        // We need to do this element by element for complex pairs
        for (size_t i = 0; i < n; ++i) {
            const size_t idx = i * 2;
            const float a_re = a_vec[idx];
            const float a_im = a_vec[idx + 1];
            const float b_re = b_vec[idx];
            const float b_im = b_vec[idx + 1];

            result_vec[idx] = a_re * b_re - a_im * b_im;     // Real part
            result_vec[idx + 1] = a_re * b_im + a_im * b_re; // Imaginary part
        }

        return VV_DSP_OK;
    } catch (...) {
        // Fall back to scalar implementation
    }
#endif

    // Scalar fallback implementation
    for (size_t i = 0; i < n; ++i) {
        const vv_dsp_real a_re = a[i].re;
        const vv_dsp_real a_im = a[i].im;
        const vv_dsp_real b_re = b[i].re;
        const vv_dsp_real b_im = b[i].im;

        result[i].re = a_re * b_re - a_im * b_im;
        result[i].im = a_re * b_im + a_im * b_re;
    }

    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_vectorized_trig_apply(
    const vv_dsp_real* in,
    vv_dsp_real* out,
    size_t n,
    int func_type
) {
    if (!in || !out || n == 0) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

    if (func_type < 0 || func_type > 2) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

#ifdef VV_DSP_USE_EIGEN
    try {
        using VectorType = Eigen::Map<const Eigen::VectorXf>;
        using MutableVectorType = Eigen::Map<Eigen::VectorXf>;

        VectorType input_vec(reinterpret_cast<const float*>(in), static_cast<Eigen::Index>(n));
        MutableVectorType output_vec(reinterpret_cast<float*>(out), static_cast<Eigen::Index>(n));

        // Apply vectorized trigonometric functions
        switch (func_type) {
            case 0: // sin
                output_vec = input_vec.array().sin();
                break;
            case 1: // cos
                output_vec = input_vec.array().cos();
                break;
            case 2: // tan
                output_vec = input_vec.array().tan();
                break;
            default:
                return VV_DSP_ERROR_OUT_OF_RANGE;
        }

        return VV_DSP_OK;
    } catch (...) {
        // Fall back to scalar implementation
    }
#endif

    // Scalar fallback implementation using vv_dsp_math.h macros
    for (size_t i = 0; i < n; ++i) {
        switch (func_type) {
            case 0: // sin
                out[i] = VV_DSP_SIN(in[i]);
                break;
            case 1: // cos
                out[i] = VV_DSP_COS(in[i]);
                break;
            case 2: // tan
                out[i] = VV_DSP_TAN(in[i]);
                break;
            default:
                return VV_DSP_ERROR_OUT_OF_RANGE;
        }
    }

    return VV_DSP_OK;
}

} // extern "C"
