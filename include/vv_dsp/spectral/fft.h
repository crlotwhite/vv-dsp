/**
 * @file fft.h
 * @brief Fast Fourier Transform (FFT) operations
 * @ingroup spectral_group
 *
 * This module provides a unified interface for Fast Fourier Transform operations
 * with support for multiple backends (FFTW, KissFFT, etc.). It supports both
 * complex-to-complex and real FFT variants with automatic memory management
 * through plan-based execution.
 */

#ifndef VV_DSP_SPECTRAL_FFT_H
#define VV_DSP_SPECTRAL_FFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"
#include <stddef.h>

/** @addtogroup spectral_group
 * @{
 */

/** @name FFT Planning and Execution
 * @{
 */

/**
 * @brief Opaque FFT plan structure
 * @details Contains backend-specific optimization data for efficient transform execution.
 * Plans should be reused when performing multiple transforms of the same size and type.
 */
typedef struct vv_dsp_fft_plan vv_dsp_fft_plan;

/**
 * @brief FFT direction specification
 * @details Specifies whether to perform forward (time→frequency) or inverse transforms
 */
typedef enum vv_dsp_fft_dir {
    VV_DSP_FFT_FORWARD = +1,   /**< Forward transform: time domain → frequency domain */
    VV_DSP_FFT_BACKWARD = -1   /**< Inverse transform: frequency domain → time domain */
} vv_dsp_fft_dir;

/**
 * @brief FFT transform type specification
 * @details Defines the input/output data types for the transform
 */
typedef enum vv_dsp_fft_type {
    VV_DSP_FFT_C2C = 0, /**< Complex → Complex transform (full spectrum) */
    VV_DSP_FFT_R2C = 1, /**< Real → Complex transform (Hermitian-packed output, size n/2+1) */
    VV_DSP_FFT_C2R = 2  /**< Complex → Real transform (Hermitian-packed input, size n/2+1) */
} vv_dsp_fft_type;

/**
 * @brief Create an FFT execution plan
 * @param n Transform length (number of input samples)
 * @param type Transform type (C2C, R2C, or C2R)
 * @param dir Transform direction (forward or backward)
 * @param out_plan Pointer to store the created plan
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Creates an optimized execution plan for FFT operations. The plan contains
 * precomputed twiddle factors and backend-specific optimizations.
 *
 * **Buffer Size Requirements:**
 * - **C2C**: input and output are both complex[n]
 * - **R2C**: input is real[n], output is complex[n/2+1] (Hermitian symmetry)
 * - **C2R**: input is complex[n/2+1] (Hermitian packed), output is real[n]
 *
 * **Scaling Convention:**
 * - Forward transforms are unscaled
 * - Backward (inverse) transforms are scaled by 1/n
 *
 * @code{.c}
 * vv_dsp_fft_plan* plan;
 * vv_dsp_status status = vv_dsp_fft_make_plan(1024, VV_DSP_FFT_C2C,
 *                                             VV_DSP_FFT_FORWARD, &plan);
 * if (status == VV_DSP_OK) {
 *     // Plan created successfully, ready for execution
 * }
 * @endcode
 *
 * @note Plans should be destroyed with vv_dsp_fft_destroy() when no longer needed
 * @see vv_dsp_fft_execute(), vv_dsp_fft_destroy()
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_make_plan(size_t n,
                                                    vv_dsp_fft_type type,
                                                    vv_dsp_fft_dir dir,
                                                    vv_dsp_fft_plan** out_plan);

/**
 * @brief Execute an FFT transform using a precomputed plan
 * @param plan FFT plan created with vv_dsp_fft_make_plan()
 * @param in Input buffer (type depends on plan configuration)
 * @param out Output buffer (type depends on plan configuration)
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Executes the FFT transform described by the plan. Input and output
 * buffers must match the types specified during plan creation.
 *
 * **Buffer Type Requirements:**
 * - **C2C**: in = vv_dsp_cpx[n], out = vv_dsp_cpx[n]
 * - **R2C**: in = vv_dsp_real[n], out = vv_dsp_cpx[n/2+1]
 * - **C2R**: in = vv_dsp_cpx[n/2+1], out = vv_dsp_real[n]
 *
 * @code{.c}
 * // Example: 1024-point complex FFT
 * vv_dsp_cpx input[1024], output[1024];
 * // ... fill input array ...
 *
 * vv_dsp_fft_plan* plan;
 * vv_dsp_fft_make_plan(1024, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);
 * vv_dsp_status status = vv_dsp_fft_execute(plan, input, output);
 *
 * // ... use output ...
 * vv_dsp_fft_destroy(plan);
 * @endcode
 *
 * @warning Input and output buffers must not overlap unless explicitly supported
 * @note This function is thread-safe if the underlying FFT backend supports it
 * @see vv_dsp_fft_make_plan(), vv_dsp_fft_destroy()
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_fft_execute(const vv_dsp_fft_plan* plan,
                                                  const void* in,
                                                  void* out);

/**
 * @brief Destroy an FFT plan and free associated resources
 * @param plan FFT plan to destroy (can be NULL)
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Frees all memory and resources associated with the FFT plan.
 * After calling this function, the plan pointer becomes invalid and should not be used.
 *
 * @code{.c}
 * vv_dsp_fft_plan* plan;
 * vv_dsp_fft_make_plan(1024, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);
 *
 * // ... use the plan ...
 *
 * vv_dsp_fft_destroy(plan);  // Clean up resources
 * plan = NULL;  // Good practice to avoid dangling pointers
 * @endcode
 *
 * @note Safe to call with NULL pointer (no-op)
 * @see vv_dsp_fft_make_plan()
 */
vv_dsp_status vv_dsp_fft_destroy(vv_dsp_fft_plan* plan);

/** @} */ // End of FFT Planning and Execution

/** @} */ // End of spectral_group

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_SPECTRAL_FFT_H
