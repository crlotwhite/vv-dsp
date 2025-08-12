/**
 * @file nan_policy.h
 * @brief NaN/Inf handling policy configuration for VV-DSP library
 * @ingroup core_group
 *
 * This file provides a configurable policy system for handling NaN (Not-a-Number)
 * and Inf (Infinity) values within the DSP library to ensure numerical stability
 * and prevent undefined behavior in production environments.
 */

#ifndef VV_DSP_CORE_NAN_POLICY_H
#define VV_DSP_CORE_NAN_POLICY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/** @addtogroup core_group
 * @{
 */

/** @name NaN/Inf Handling Policy
 * @{
 */

/**
 * @brief NaN/Inf handling policy enumeration
 * @details Defines how the library should handle NaN and Infinity values
 * encountered during DSP operations.
 */
typedef enum vv_dsp_nan_policy {
    VV_DSP_NAN_POLICY_PROPAGATE = 0, /**< Default: Let NaN/Inf values pass through calculations */
    VV_DSP_NAN_POLICY_IGNORE = 1,    /**< Replace NaN/Inf with a neutral value (0.0) */
    VV_DSP_NAN_POLICY_ERROR = 2,     /**< Return an error code immediately upon detecting NaN/Inf */
    VV_DSP_NAN_POLICY_CLAMP = 3      /**< Replace +/-Inf with max/min finite values, and NaN with 0.0 */
} vv_dsp_nan_policy_e;

/**
 * @brief Set the global policy for handling NaN/Inf values
 * @param policy The policy to use for handling NaN/Inf values
 * @details This function sets the global policy that will be applied by all
 * DSP functions when they encounter NaN or Infinity values. The setting is
 * thread-local if the library is compiled with thread support.
 *
 * @code{.c}
 * // Set policy to replace NaN/Inf with 0.0
 * vv_dsp_set_nan_policy(VV_DSP_NAN_POLICY_IGNORE);
 * 
 * // Set policy to return error on NaN/Inf
 * vv_dsp_set_nan_policy(VV_DSP_NAN_POLICY_ERROR);
 * @endcode
 */
void vv_dsp_set_nan_policy(vv_dsp_nan_policy_e policy);

/**
 * @brief Retrieve the current global policy for handling NaN/Inf values
 * @return The currently active NaN/Inf handling policy
 * @details Returns the policy that is currently in effect for the calling thread.
 * The default policy is VV_DSP_NAN_POLICY_PROPAGATE.
 *
 * @code{.c}
 * vv_dsp_nan_policy_e current_policy = vv_dsp_get_nan_policy();
 * if (current_policy == VV_DSP_NAN_POLICY_ERROR) {
 *     // Handle error policy case
 * }
 * @endcode
 */
vv_dsp_nan_policy_e vv_dsp_get_nan_policy(void);

/** @name Internal Helper Functions
 * @brief Internal functions for applying NaN/Inf policies to arrays
 * @details These functions are used internally by DSP functions to apply
 * the current NaN/Inf policy. They are exposed here for potential use by
 * advanced users or module implementers.
 * @{
 */

/**
 * @brief Apply the current NaN/Inf policy to an array in-place
 * @param data Pointer to the array data
 * @param len Number of elements in the array
 * @return VV_DSP_OK on success, VV_DSP_ERROR_NAN_INF if policy is ERROR and NaN/Inf found
 * @details This function modifies the input array according to the current policy.
 * For PROPAGATE policy, no modifications are made. For IGNORE policy, NaN/Inf values
 * are replaced with 0.0. For CLAMP policy, +/-Inf are replaced with max/min finite
 * values and NaN with 0.0. For ERROR policy, returns an error if any NaN/Inf is found.
 */
vv_dsp_status vv_dsp_apply_nan_policy_inplace(vv_dsp_real* data, size_t len);

/**
 * @brief Check array for NaN/Inf values and apply policy (const version)
 * @param data Pointer to the input array data (const)
 * @param len Number of elements in the array
 * @param output Optional pointer to output array (if NULL, only check is performed)
 * @return VV_DSP_OK on success, VV_DSP_ERROR_NAN_INF if policy is ERROR and NaN/Inf found
 * @details This function can either just check for NaN/Inf values (if output is NULL)
 * or copy the input to output while applying the policy. This is useful for functions
 * that need to process const input arrays.
 */
vv_dsp_status vv_dsp_apply_nan_policy_copy(const vv_dsp_real* data, size_t len, vv_dsp_real* output);

/** @} */

/** @} */
/** @} */ // End of core_group

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_CORE_NAN_POLICY_H
