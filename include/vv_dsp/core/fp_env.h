/**
 * @file fp_env.h
 * @brief Floating-point environment control for denormal handling
 *
 * This module provides platform-specific control over floating-point
 * denormal handling to maintain real-time performance in DSP applications.
 * Denormal numbers can cause significant performance penalties on some
 * architectures, and this API allows enabling Flush-to-Zero (FTZ) and
 * Denormals-are-Zero (DAZ) modes to mitigate these issues.
 */

#ifndef VV_DSP_CORE_FP_ENV_H
#define VV_DSP_CORE_FP_ENV_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enables or disables flush-to-zero mode for the current thread.
 *
 * When enabled, denormal inputs are treated as zero (DAZ) and denormal
 * results are flushed to zero (FTZ). This prevents performance penalties
 * from subnormal numbers in real-time audio processing.
 *
 * @note This setting affects only the current thread and is not inherited
 *       by child threads. The setting persists until explicitly changed
 *       or the thread terminates.
 *
 * @note On unsupported platforms, this function does nothing and returns
 *       immediately without error.
 *
 * @param enable True to enable FTZ/DAZ, false to disable and restore
 *               normal IEEE 754 behavior
 */
void vv_dsp_set_flush_denormals(bool enable);

/**
 * @brief Gets the current flush-to-zero mode for the thread.
 *
 * @return True if FTZ/DAZ is enabled, false otherwise.
 *         On unsupported platforms, always returns false.
 */
bool vv_dsp_get_flush_denormals_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* VV_DSP_CORE_FP_ENV_H */
