/**
 * @file vv_dsp_vectorized_math_fallback.c
 * @brief Fallback scalar implementations for vectorized math operations
 */

#include "vv_dsp/core/vv_dsp_vectorized_math.h"
#include "vv_dsp/vv_dsp_math.h"

int vv_dsp_vectorized_math_available(void) {
    return 0; /* No vectorization available, using scalar fallback */
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

    /* Scalar implementation */
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

    /* Scalar complex multiplication implementation */
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

    /* Scalar implementation using vv_dsp_math.h macros */
    for (size_t i = 0; i < n; ++i) {
        switch (func_type) {
            case 0: /* sin */
                out[i] = VV_DSP_SIN(in[i]);
                break;
            case 1: /* cos */
                out[i] = VV_DSP_COS(in[i]);
                break;
            case 2: /* tan */
                out[i] = VV_DSP_TAN(in[i]);
                break;
            default:
                return VV_DSP_ERROR_OUT_OF_RANGE;
        }
    }

    return VV_DSP_OK;
}
