/**
 * @file nan_policy.c
 * @brief Implementation of NaN/Inf handling policy for VV-DSP library
 */

#include "vv_dsp/core/nan_policy.h"
#include <math.h>
#include <float.h>

// Thread-local storage for the NaN policy
// For MSVC, use __declspec(thread)
// For C11 compliant compilers, use _Thread_local
// For older compilers or non-threaded environments, use static global
#if defined(_MSC_VER)
    static __declspec(thread) vv_dsp_nan_policy_e g_nan_policy = VV_DSP_NAN_POLICY_PROPAGATE;
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__STDC_NO_THREADS__)
    static _Thread_local vv_dsp_nan_policy_e g_nan_policy = VV_DSP_NAN_POLICY_PROPAGATE;
#else
    // Fallback to static global for non-threaded environments or older compilers
    static vv_dsp_nan_policy_e g_nan_policy = VV_DSP_NAN_POLICY_PROPAGATE;
#endif

void vv_dsp_set_nan_policy(vv_dsp_nan_policy_e policy) {
    // Validate policy value
    if (policy >= VV_DSP_NAN_POLICY_PROPAGATE && policy <= VV_DSP_NAN_POLICY_CLAMP) {
        g_nan_policy = policy;
    }
}

vv_dsp_nan_policy_e vv_dsp_get_nan_policy(void) {
    return g_nan_policy;
}

/**
 * @brief Internal helper to apply NaN/Inf policy to a single value
 * @param value Pointer to the value to check and potentially modify
 * @param policy The policy to apply
 * @return VV_DSP_OK on success, VV_DSP_ERROR_NAN_INF if policy is ERROR and NaN/Inf found
 */
static inline vv_dsp_status vv_dsp_apply_nan_policy_single(vv_dsp_real* value, vv_dsp_nan_policy_e policy) {
    if (!isnan(*value) && !isinf(*value)) {
        return VV_DSP_OK;  // Value is finite, no action needed
    }

    switch (policy) {
        case VV_DSP_NAN_POLICY_PROPAGATE:
            // Do nothing - let NaN/Inf pass through
            return VV_DSP_OK;

        case VV_DSP_NAN_POLICY_IGNORE:
            // Replace NaN/Inf with 0.0
            *value = (vv_dsp_real)0.0;
            return VV_DSP_OK;

        case VV_DSP_NAN_POLICY_ERROR:
            // Return error immediately
            return VV_DSP_ERROR_NAN_INF;

        case VV_DSP_NAN_POLICY_CLAMP:
            if (isnan(*value)) {
                *value = (vv_dsp_real)0.0;
            } else if (isinf(*value)) {
                if (*value > 0) {
#ifdef VV_DSP_USE_DOUBLE
                    *value = DBL_MAX;
#else
                    *value = FLT_MAX;
#endif
                } else {
#ifdef VV_DSP_USE_DOUBLE
                    *value = -DBL_MAX;
#else
                    *value = -FLT_MAX;
#endif
                }
            }
            return VV_DSP_OK;

        default:
            // Unknown policy, treat as PROPAGATE
            return VV_DSP_OK;
    }
}

/**
 * @brief Apply the current NaN/Inf policy to an array in-place
 * @param data Pointer to the array data
 * @param len Number of elements in the array
 * @return VV_DSP_OK on success, VV_DSP_ERROR_NAN_INF if policy is ERROR and NaN/Inf found
 */
vv_dsp_status vv_dsp_apply_nan_policy_inplace(vv_dsp_real* data, size_t len) {
    if (!data) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

    if (len == 0) {
        return VV_DSP_OK;
    }

    vv_dsp_nan_policy_e policy = vv_dsp_get_nan_policy();

    // For PROPAGATE policy, we can skip the entire check
    if (policy == VV_DSP_NAN_POLICY_PROPAGATE) {
        return VV_DSP_OK;
    }

    for (size_t i = 0; i < len; i++) {
        vv_dsp_status status = vv_dsp_apply_nan_policy_single(&data[i], policy);
        if (status != VV_DSP_OK) {
            return status;  // Early return on error
        }
    }

    return VV_DSP_OK;
}

/**
 * @brief Check array for NaN/Inf values and apply policy (const version)
 * @param data Pointer to the input array data (const)
 * @param len Number of elements in the array
 * @param output Optional pointer to output array (if NULL, only check is performed)
 * @return VV_DSP_OK on success, VV_DSP_ERROR_NAN_INF if policy is ERROR and NaN/Inf found
 */
vv_dsp_status vv_dsp_apply_nan_policy_copy(const vv_dsp_real* data, size_t len, vv_dsp_real* output) {
    if (!data) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

    if (len == 0) {
        return VV_DSP_OK;
    }

    vv_dsp_nan_policy_e policy = vv_dsp_get_nan_policy();

    // For PROPAGATE policy, we can skip processing if output is provided
    if (policy == VV_DSP_NAN_POLICY_PROPAGATE) {
        if (output) {
            // Just copy the data as-is
            for (size_t i = 0; i < len; i++) {
                output[i] = data[i];
            }
        }
        return VV_DSP_OK;
    }

    for (size_t i = 0; i < len; i++) {
        vv_dsp_real value = data[i];

        if (isnan(value) || isinf(value)) {
            switch (policy) {
                case VV_DSP_NAN_POLICY_IGNORE:
                    value = (vv_dsp_real)0.0;
                    break;

                case VV_DSP_NAN_POLICY_ERROR:
                    return VV_DSP_ERROR_NAN_INF;

                case VV_DSP_NAN_POLICY_CLAMP:
                    if (isnan(value)) {
                        value = (vv_dsp_real)0.0;
                    } else if (isinf(value)) {
                        if (value > 0) {
#ifdef VV_DSP_USE_DOUBLE
                            value = DBL_MAX;
#else
                            value = FLT_MAX;
#endif
                        } else {
#ifdef VV_DSP_USE_DOUBLE
                            value = -DBL_MAX;
#else
                            value = -FLT_MAX;
#endif
                        }
                    }
                    break;

                default:
                    // Unknown policy, leave value unchanged
                    break;
            }
        }

        if (output) {
            output[i] = value;
        }
    }

    return VV_DSP_OK;
}
