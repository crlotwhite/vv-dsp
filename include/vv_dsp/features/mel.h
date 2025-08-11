#ifndef VV_DSP_FEATURES_MEL_H
#define VV_DSP_FEATURES_MEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/spectral/dct.h"

// Mel scale variants
typedef enum vv_dsp_mel_variant {
    VV_DSP_MEL_VARIANT_HTK = 0,     // HTK variant (more traditional)
    VV_DSP_MEL_VARIANT_SLANEY = 1   // Slaney variant (more linear)
} vv_dsp_mel_variant;

// Opaque MFCC context/plan
typedef struct vv_dsp_mfcc_plan vv_dsp_mfcc_plan;

// --------------- Mel Scale Conversion Functions ---------------

/**
 * Convert frequency in Hz to Mel scale
 * @param hz Frequency in Hertz (must be non-negative)
 * @return Frequency in Mel scale
 */
vv_dsp_real vv_dsp_hz_to_mel(vv_dsp_real hz);

/**
 * Convert frequency in Mel scale to Hz
 * @param mel Frequency in Mel scale (must be non-negative)
 * @return Frequency in Hertz
 */
vv_dsp_real vv_dsp_mel_to_hz(vv_dsp_real mel);

// --------------- Mel Filterbank Generation ---------------

/**
 * Create triangular Mel filterbank weights
 * @param n_fft FFT size (number of FFT bins = n_fft/2 + 1 for real input)
 * @param n_mels Number of Mel filters to generate
 * @param sample_rate Sample rate in Hz
 * @param fmin Minimum frequency in Hz
 * @param fmax Maximum frequency in Hz (typically sample_rate/2)
 * @param variant Mel scale variant (HTK or Slaney)
 * @param out_filterbank_weights Pointer to allocated filterbank matrix (n_mels x n_fft_bins)
 * @param out_num_filters Number of filters created (should equal n_mels)
 * @param out_filter_len Length of each filter (number of FFT bins = n_fft/2 + 1)
 * @return VV_DSP_OK on success, error code otherwise
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_mel_filterbank_create(
    size_t n_fft,
    size_t n_mels,
    vv_dsp_real sample_rate,
    vv_dsp_real fmin,
    vv_dsp_real fmax,
    vv_dsp_mel_variant variant,
    vv_dsp_real** out_filterbank_weights,
    size_t* out_num_filters,
    size_t* out_filter_len
);

/**
 * Free filterbank weights allocated by vv_dsp_mel_filterbank_create
 * @param filterbank_weights Filterbank weights to free
 * @param n_mels Number of Mel filters
 */
void vv_dsp_mel_filterbank_free(vv_dsp_real* filterbank_weights, size_t n_mels);

// --------------- Log-Mel Spectrogram Computation ---------------

/**
 * Compute log-Mel spectrogram from power spectrogram
 * @param power_spectrogram Input power spectrogram (num_frames x n_fft_bins)
 * @param num_frames Number of time frames
 * @param n_fft_bins Number of FFT bins per frame (n_fft/2 + 1)
 * @param filterbank_weights Mel filterbank weights (n_mels x n_fft_bins)
 * @param n_mels Number of Mel filters
 * @param log_epsilon Small value added before log to avoid log(0)
 * @param out_log_mel_spectrogram Output log-Mel spectrogram (num_frames x n_mels)
 * @return VV_DSP_OK on success, error code otherwise
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_compute_log_mel_spectrogram(
    const vv_dsp_real* power_spectrogram,
    size_t num_frames,
    size_t n_fft_bins,
    const vv_dsp_real* filterbank_weights,
    size_t n_mels,
    vv_dsp_real log_epsilon,
    vv_dsp_real* out_log_mel_spectrogram
);

// --------------- MFCC Computation ---------------

/**
 * Compute MFCC coefficients from log-Mel spectrogram
 * @param log_mel_spectrogram Input log-Mel spectrogram (num_frames x n_mels)
 * @param num_frames Number of time frames
 * @param n_mels Number of Mel filter banks
 * @param num_mfcc_coeffs Number of MFCC coefficients to compute (should be <= n_mels)
 * @param dct_type DCT type (should be VV_DSP_DCT_II for MFCC)
 * @param lifter_coeff Liftering coefficient (0 = no liftering)
 * @param out_mfcc_coeffs Output MFCC coefficients (num_frames x num_mfcc_coeffs)
 * @return VV_DSP_OK on success, error code otherwise
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_mfcc(
    const vv_dsp_real* log_mel_spectrogram,
    size_t num_frames,
    size_t n_mels,
    size_t num_mfcc_coeffs,
    vv_dsp_dct_type dct_type,
    vv_dsp_real lifter_coeff,
    vv_dsp_real* out_mfcc_coeffs
);

// --------------- MFCC Context Management ---------------

/**
 * Initialize MFCC plan/context with pre-computed resources
 * @param n_fft FFT size
 * @param n_mels Number of Mel filters
 * @param num_mfcc_coeffs Number of MFCC coefficients
 * @param sample_rate Sample rate in Hz
 * @param fmin Minimum frequency in Hz
 * @param fmax Maximum frequency in Hz
 * @param variant Mel scale variant
 * @param dct_type DCT type (should be VV_DSP_DCT_II for MFCC)
 * @param lifter_coeff Liftering coefficient
 * @param log_epsilon Small value for log computation
 * @param out_plan Pointer to created MFCC plan
 * @return VV_DSP_OK on success, error code otherwise
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_mfcc_init(
    size_t n_fft,
    size_t n_mels,
    size_t num_mfcc_coeffs,
    vv_dsp_real sample_rate,
    vv_dsp_real fmin,
    vv_dsp_real fmax,
    vv_dsp_mel_variant variant,
    vv_dsp_dct_type dct_type,
    vv_dsp_real lifter_coeff,
    vv_dsp_real log_epsilon,
    vv_dsp_mfcc_plan** out_plan
);

/**
 * Process power spectrogram through complete MFCC pipeline
 * @param plan MFCC plan/context
 * @param power_spectrogram Input power spectrogram (num_frames x n_fft_bins)
 * @param num_frames Number of time frames
 * @param out_mfcc_coeffs Output MFCC coefficients (num_frames x num_mfcc_coeffs)
 * @return VV_DSP_OK on success, error code otherwise
 */
VV_DSP_NODISCARD vv_dsp_status vv_dsp_mfcc_process(
    const vv_dsp_mfcc_plan* plan,
    const vv_dsp_real* power_spectrogram,
    size_t num_frames,
    vv_dsp_real* out_mfcc_coeffs
);

/**
 * Destroy MFCC plan and free all resources
 * @param plan MFCC plan to destroy
 * @return VV_DSP_OK on success, error code otherwise
 */
vv_dsp_status vv_dsp_mfcc_destroy(vv_dsp_mfcc_plan* plan);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_FEATURES_MEL_H
