/**
 * @file core.h
 * @brief Core DSP utilities and mathematical operations
 * @ingroup core_group
 *
 * This file provides fundamental DSP operations including complex number arithmetic,
 * array-based statistics, and basic mathematical utilities.
 */

#ifndef VV_DSP_CORE_H
#define VV_DSP_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/** @addtogroup core_group
 * @{
 */

/** @name Basic Functions
 * @{
 */

/**
 * @brief Simple integer addition function
 * @param a First integer
 * @param b Second integer
 * @return Sum of a and b
 * @note This function is primarily used for testing purposes
 */
int vv_dsp_add_int(int a, int b);
/** @} */

/** @name Complex Number Operations
 * @{
 */

/**
 * @brief Add two complex numbers
 * @param a First complex number
 * @param b Second complex number
 * @return Complex sum a + b
 *
 * @code{.c}
 * vv_dsp_cpx a = vv_dsp_cpx_make(1.0, 2.0);  // 1+2i
 * vv_dsp_cpx b = vv_dsp_cpx_make(3.0, 4.0);  // 3+4i
 * vv_dsp_cpx result = vv_dsp_cpx_add(a, b);   // 4+6i
 * @endcode
 */
vv_dsp_cpx vv_dsp_cpx_add(vv_dsp_cpx a, vv_dsp_cpx b);

/**
 * @brief Multiply two complex numbers
 * @param a First complex number
 * @param b Second complex number
 * @return Complex product a * b
 *
 * @details Computes (a.re + a.im*i) * (b.re + b.im*i) =
 *          (a.re*b.re - a.im*b.im) + (a.re*b.im + a.im*b.re)*i
 *
 * @code{.c}
 * vv_dsp_cpx a = vv_dsp_cpx_make(1.0, 2.0);  // 1+2i
 * vv_dsp_cpx b = vv_dsp_cpx_make(3.0, 4.0);  // 3+4i
 * vv_dsp_cpx result = vv_dsp_cpx_mul(a, b);   // -5+10i
 * @endcode
 */
vv_dsp_cpx vv_dsp_cpx_mul(vv_dsp_cpx a, vv_dsp_cpx b);

/**
 * @brief Compute complex conjugate
 * @param z Input complex number
 * @return Complex conjugate of z (real part unchanged, imaginary part negated)
 *
 * @code{.c}
 * vv_dsp_cpx z = vv_dsp_cpx_make(3.0, 4.0);     // 3+4i
 * vv_dsp_cpx conj_z = vv_dsp_cpx_conj(z);       // 3-4i
 * @endcode
 */
vv_dsp_cpx vv_dsp_cpx_conj(vv_dsp_cpx z);

/**
 * @brief Compute magnitude (absolute value) of complex number
 * @param z Input complex number
 * @return Magnitude |z| = sqrt(re² + im²)
 *
 * @code{.c}
 * vv_dsp_cpx z = vv_dsp_cpx_make(3.0, 4.0);     // 3+4i
 * vv_dsp_real mag = vv_dsp_cpx_abs(z);          // 5.0
 * @endcode
 */
vv_dsp_real vv_dsp_cpx_abs(vv_dsp_cpx z);

/**
 * @brief Compute phase (argument) of complex number
 * @param z Input complex number
 * @return Phase angle in radians, range [-π, π]
 *
 * @details Uses atan2(im, re) to compute the phase angle
 *
 * @code{.c}
 * vv_dsp_cpx z = vv_dsp_cpx_make(1.0, 1.0);     // 1+1i
 * vv_dsp_real phase = vv_dsp_cpx_phase(z);      // π/4 (45 degrees)
 * @endcode
 */
vv_dsp_real vv_dsp_cpx_phase(vv_dsp_cpx z);

/**
 * @brief Create complex number from polar coordinates
 * @param r Magnitude (radius)
 * @param theta Phase angle in radians
 * @return Complex number z = r * e^(i*theta) = r * (cos(theta) + i*sin(theta))
 *
 * @code{.c}
 * vv_dsp_real r = 5.0;
 * vv_dsp_real theta = M_PI / 4;  // 45 degrees
 * vv_dsp_cpx z = vv_dsp_cpx_from_polar(r, theta);  // 5*(cos(π/4) + i*sin(π/4))
 * @endcode
 */
vv_dsp_cpx vv_dsp_cpx_from_polar(vv_dsp_real r, vv_dsp_real theta);
/** @} */

/** @name Array Statistics
 * @{
 */

/**
 * @brief Compute sum of array elements
 * @param x Input array
 * @param n Number of elements
 * @param out Pointer to store the result
 * @return VV_DSP_OK on success, error code on failure
 *
 * @code{.c}
 * vv_dsp_real data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
 * vv_dsp_real sum;
 * vv_dsp_status status = vv_dsp_sum(data, 5, &sum);  // sum = 15.0
 * @endcode
 */
vv_dsp_status vv_dsp_sum(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Compute arithmetic mean of array elements
 * @param x Input array
 * @param n Number of elements
 * @param out Pointer to store the result
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Computes mean = sum(x) / n
 *
 * @code{.c}
 * vv_dsp_real data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
 * vv_dsp_real mean;
 * vv_dsp_status status = vv_dsp_mean(data, 5, &mean);  // mean = 3.0
 * @endcode
 */
vv_dsp_status vv_dsp_mean(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Compute variance of array elements
 * @param x Input array
 * @param n Number of elements
 * @param out Pointer to store the result
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Computes population variance: var = sum((x[i] - mean)²) / n
 *
 * @code{.c}
 * vv_dsp_real data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
 * vv_dsp_real variance;
 * vv_dsp_status status = vv_dsp_var(data, 5, &variance);  // variance = 2.0
 * @endcode
 */
vv_dsp_status vv_dsp_var(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Find minimum value in array
 * @param x Input array
 * @param n Number of elements
 * @param out Pointer to store the minimum value
 * @return VV_DSP_OK on success, error code on failure
 *
 * @code{.c}
 * vv_dsp_real data[] = {3.0, 1.0, 4.0, 1.0, 5.0};
 * vv_dsp_real min_val;
 * vv_dsp_status status = vv_dsp_min(data, 5, &min_val);  // min_val = 1.0
 * @endcode
 */
vv_dsp_status vv_dsp_min(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Find maximum value in array
 * @param x Input array
 * @param n Number of elements
 * @param out Pointer to store the maximum value
 * @return VV_DSP_OK on success, error code on failure
 *
 * @code{.c}
 * vv_dsp_real data[] = {3.0, 1.0, 4.0, 1.0, 5.0};
 * vv_dsp_real max_val;
 * vv_dsp_status status = vv_dsp_max(data, 5, &max_val);  // max_val = 5.0
 * @endcode
 */
vv_dsp_status vv_dsp_max(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Find index of minimum value (argmin)
 * @param x Input array
 * @param n Number of elements
 * @param idx Pointer to store the index of minimum value
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details If multiple elements have the same minimum value, returns the first occurrence
 *
 * @code{.c}
 * vv_dsp_real data[] = {3.0, 1.0, 4.0, 1.0, 5.0};
 * size_t min_idx;
 * vv_dsp_status status = vv_dsp_argmin(data, 5, &min_idx);  // min_idx = 1
 * @endcode
 */
vv_dsp_status vv_dsp_argmin(const vv_dsp_real* x, size_t n, size_t* idx);

/**
 * @brief Find index of maximum value (argmax)
 * @param x Input array
 * @param n Number of elements
 * @param idx Pointer to store the index of maximum value
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details If multiple elements have the same maximum value, returns the first occurrence
 *
 * @code{.c}
 * vv_dsp_real data[] = {3.0, 1.0, 4.0, 1.0, 5.0};
 * size_t max_idx;
 * vv_dsp_status status = vv_dsp_argmax(data, 5, &max_idx);  // max_idx = 4
 * @endcode
 */
vv_dsp_status vv_dsp_argmax(const vv_dsp_real* x, size_t n, size_t* idx);
/** @} */

/** @name Array Utilities
 * @{
 */

/**
 * @brief Compute cumulative sum of array elements
 * @param x Input array of length n
 * @param n Number of elements
 * @param y Output array of length n (can be same as x for in-place operation)
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Computes y[i] = sum(x[0:i]) for i = 0, 1, ..., n-1
 *
 * @code{.c}
 * vv_dsp_real data[] = {1.0, 2.0, 3.0, 4.0};
 * vv_dsp_real cumsum[4];
 * vv_dsp_status status = vv_dsp_cumsum(data, 4, cumsum);  // {1.0, 3.0, 6.0, 10.0}
 * @endcode
 */
vv_dsp_status vv_dsp_cumsum(const vv_dsp_real* x, size_t n, vv_dsp_real* y);

/**
 * @brief Compute discrete difference of array elements
 * @param x Input array of length n
 * @param n Number of input elements
 * @param y Output array of length n-1
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Computes y[i] = x[i+1] - x[i] for i = 0, 1, ..., n-2
 *
 * @code{.c}
 * vv_dsp_real data[] = {1.0, 3.0, 6.0, 10.0};
 * vv_dsp_real diff[3];
 * vv_dsp_status status = vv_dsp_diff(data, 4, diff);  // {2.0, 3.0, 4.0}
 * @endcode
 */
vv_dsp_status vv_dsp_diff(const vv_dsp_real* x, size_t n, vv_dsp_real* y);

/**
 * @brief Clamp value to specified range
 * @param v Input value
 * @param lo Lower bound (inclusive)
 * @param hi Upper bound (inclusive)
 * @return Clamped value: max(lo, min(hi, v))
 *
 * @code{.c}
 * vv_dsp_real result = vv_dsp_clamp(15.0, 0.0, 10.0);  // result = 10.0
 * vv_dsp_real result2 = vv_dsp_clamp(-5.0, 0.0, 10.0); // result2 = 0.0
 * vv_dsp_real result3 = vv_dsp_clamp(5.0, 0.0, 10.0);  // result3 = 5.0
 * @endcode
 */
vv_dsp_real vv_dsp_clamp(vv_dsp_real v, vv_dsp_real lo, vv_dsp_real hi);

/**
 * @brief Flush denormal floating-point numbers to zero
 * @details Sets processor flags to treat denormal numbers as zero for better performance
 * @note This is a platform-specific optimization and may not be available on all systems
 */
void vv_dsp_flush_denormals(void);
/** @} */

/** @name Advanced Statistics and Signal Measurements
 * @brief Functions for signal analysis and statistical measurements
 * @{
 */

/**
 * @brief Compute Root Mean Square (RMS) value
 * @param x Input signal array
 * @param n Number of samples
 * @param out Pointer to store the RMS value
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Computes RMS = sqrt(mean(x²)) = sqrt(sum(x[i]²)/n)
 *
 * @code{.c}
 * vv_dsp_real signal[] = {1.0, -1.0, 2.0, -2.0};
 * vv_dsp_real rms;
 * vv_dsp_status status = vv_dsp_rms(signal, 4, &rms);  // rms ≈ 1.58
 * @endcode
 */
vv_dsp_status vv_dsp_rms(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Find peak values (minimum and maximum)
 * @param x Input signal array
 * @param n Number of samples
 * @param min_val Pointer to store minimum value (can be NULL)
 * @param max_val Pointer to store maximum value (can be NULL)
 * @return VV_DSP_OK on success, error code on failure
 *
 * @code{.c}
 * vv_dsp_real signal[] = {1.0, -3.0, 2.0, -1.0, 4.0};
 * vv_dsp_real min_peak, max_peak;
 * vv_dsp_status status = vv_dsp_peak(signal, 5, &min_peak, &max_peak);  // min=-3.0, max=4.0
 * @endcode
 */
vv_dsp_status vv_dsp_peak(const vv_dsp_real* x, size_t n, vv_dsp_real* min_val, vv_dsp_real* max_val);

/**
 * @brief Compute crest factor (peak-to-RMS ratio)
 * @param x Input signal array
 * @param n Number of samples
 * @param out Pointer to store the crest factor
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Crest factor = max(|x|) / RMS(x). Higher values indicate more dynamic range.
 *
 * @code{.c}
 * vv_dsp_real signal[] = {0.1, 0.1, 0.1, 1.0};  // One large peak
 * vv_dsp_real crest;
 * vv_dsp_status status = vv_dsp_crest_factor(signal, 4, &crest);  // High crest factor
 * @endcode
 */
vv_dsp_status vv_dsp_crest_factor(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Compute zero crossing rate
 * @param x Input signal array
 * @param n Number of samples
 * @param count_out Pointer to store the number of zero crossings
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Counts the number of times the signal changes sign between consecutive samples.
 * Useful for pitch estimation and signal analysis.
 *
 * @code{.c}
 * vv_dsp_real signal[] = {1.0, -1.0, 1.0, -1.0, 1.0};  // Alternating signal
 * size_t crossings;
 * vv_dsp_status status = vv_dsp_zero_crossing_rate(signal, 5, &crossings);  // crossings = 4
 * @endcode
 */
vv_dsp_status vv_dsp_zero_crossing_rate(const vv_dsp_real* x, size_t n, size_t* count_out);

/**
 * @brief Compute skewness (third moment about the mean)
 * @param x Input signal array
 * @param n Number of samples
 * @param out Pointer to store the skewness value
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Skewness measures the asymmetry of the distribution:
 * - Skewness = 0: symmetric distribution
 * - Skewness > 0: right tail is longer
 * - Skewness < 0: left tail is longer
 */
vv_dsp_status vv_dsp_skewness(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Compute kurtosis (fourth moment about the mean)
 * @param x Input signal array
 * @param n Number of samples
 * @param out Pointer to store the kurtosis value
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Kurtosis measures the "tailedness" of the distribution:
 * - Kurtosis = 3: normal distribution (mesokurtic)
 * - Kurtosis > 3: heavy tails (leptokurtic)
 * - Kurtosis < 3: light tails (platykurtic)
 */
vv_dsp_status vv_dsp_kurtosis(const vv_dsp_real* x, size_t n, vv_dsp_real* out);

/**
 * @brief Compute autocorrelation function
 * @param x Input signal array
 * @param n Number of input samples
 * @param r Output autocorrelation array
 * @param r_len Length of output array (number of lags to compute)
 * @param biased Use biased (1) or unbiased (0) estimator
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Computes R[k] = sum(x[i] * x[i+k]) for k = 0, 1, ..., r_len-1
 *
 * @code{.c}
 * vv_dsp_real signal[100];
 * vv_dsp_real autocorr[50];
 * // Fill signal with data...
 * vv_dsp_status status = vv_dsp_autocorrelation(signal, 100, autocorr, 50, 0);
 * @endcode
 */
vv_dsp_status vv_dsp_autocorrelation(const vv_dsp_real* x, size_t n, vv_dsp_real* r, size_t r_len, int biased);

/**
 * @brief Compute cross-correlation between two signals
 * @param x First input signal array
 * @param nx Number of samples in first signal
 * @param y Second input signal array
 * @param ny Number of samples in second signal
 * @param r Output cross-correlation array
 * @param r_len Length of output array
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Computes cross-correlation R_xy[k] = sum(x[i] * y[i+k])
 * Useful for signal alignment, pattern matching, and delay estimation.
 *
 * @code{.c}
 * vv_dsp_real signal1[100], signal2[100];
 * vv_dsp_real xcorr[199];  // Length up to nx + ny - 1
 * // Fill signals with data...
 * vv_dsp_status status = vv_dsp_cross_correlation(signal1, 100, signal2, 100, xcorr, 199);
 * @endcode
 */
vv_dsp_status vv_dsp_cross_correlation(const vv_dsp_real* x, size_t nx,
									   const vv_dsp_real* y, size_t ny,
									   vv_dsp_real* r, size_t r_len);
/** @} */

/** @name Signal Framing and Overlap-Add
 * @{
 */

/**
 * @brief Calculate the number of frames for a given signal and framing parameters
 * @param signal_len Total number of samples in the signal
 * @param frame_len Length of each frame (e.g., FFT size)
 * @param hop_len Number of samples to advance between frames
 * @param center If non-zero, use centered framing mode
 * @return Number of frames that will be generated
 *
 * @details For centered framing (center != 0), the number of frames is signal_len / hop_len.
 * For non-centered framing (center == 0), it is 1 + (signal_len - frame_len) / hop_len
 * when signal_len >= frame_len, otherwise 0.
 *
 * @code{.c}
 * size_t num_frames = vv_dsp_get_num_frames(1024, 256, 128, 1);
 * @endcode
 */
size_t vv_dsp_get_num_frames(size_t signal_len, size_t frame_len, size_t hop_len, int center);

/**
 * @brief Extract a single frame from an input signal
 * @param signal Pointer to the input signal buffer
 * @param signal_len Total number of samples in the signal
 * @param frame_buffer Pre-allocated output buffer of size frame_len
 * @param frame_len The length of each frame (e.g., FFT size)
 * @param hop_len The number of samples to advance between frames
 * @param frame_index The index of the frame to extract
 * @param center If non-zero, use centered framing with reflection padding
 * @param window Optional pointer to a windowing array of size frame_len. If NULL, no windowing is applied
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Extracts a single, optionally windowed and padded, frame from an input signal.
 * For centered framing, reflection padding is used for frames near signal boundaries.
 * If a window is provided, it is applied via element-wise multiplication.
 *
 * @code{.c}
 * vv_dsp_real signal[1024];
 * vv_dsp_real frame[256];
 * vv_dsp_real window[256];
 * // Fill signal and window...
 * int status = vv_dsp_fetch_frame(signal, 1024, frame, 256, 128, 0, 1, window);
 * @endcode
 */
vv_dsp_status vv_dsp_fetch_frame(const vv_dsp_real* signal, size_t signal_len,
                                 vv_dsp_real* frame_buffer, size_t frame_len,
                                 size_t hop_len, size_t frame_index, int center,
                                 const vv_dsp_real* window);

/**
 * @brief Add a frame into an output buffer using overlap-add
 * @param frame The processed frame to be added back
 * @param output_signal The buffer for the reconstructed signal
 * @param output_len The total length of the output signal buffer
 * @param frame_len Length of the frame
 * @param hop_len Number of samples to advance between frames
 * @param frame_index The index of the frame
 * @return VV_DSP_OK on success, error code on failure
 *
 * @details Adds a frame into an output buffer at the correct position, overlapping
 * with previous frames. This assumes that a proper synthesis window has already
 * been applied to the frame if necessary for perfect reconstruction.
 *
 * @code{.c}
 * vv_dsp_real processed_frame[256];
 * vv_dsp_real output[1024];
 * // Process frame...
 * int status = vv_dsp_overlap_add(processed_frame, output, 1024, 256, 128, 0);
 * @endcode
 */
vv_dsp_status vv_dsp_overlap_add(const vv_dsp_real* frame, vv_dsp_real* output_signal,
                                 size_t output_len, size_t frame_len, size_t hop_len,
                                 size_t frame_index);

/** @} */

/** @} */ // End of core_group

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_CORE_H
