#include "vv_dsp/features/mel.h"
#include "vv_dsp/vv_dsp_math.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Math constants if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --------------- Mel Scale Conversion Functions ---------------

vv_dsp_real vv_dsp_hz_to_mel(vv_dsp_real hz) {
    // HTK variant: mel = 2595 * log10(1 + hz/700)
    if (hz < 0.0f) {
        return 0.0f;  // Handle negative input gracefully
    }
    return 2595.0f * log10f(1.0f + hz / 700.0f);
}

vv_dsp_real vv_dsp_mel_to_hz(vv_dsp_real mel) {
    // HTK variant: hz = 700 * (10^(mel/2595) - 1)
    if (mel < 0.0f) {
        return 0.0f;  // Handle negative input gracefully
    }
    return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
}

// --------------- Helper Functions ---------------

/**
 * Create linearly spaced array
 */
static void linspace(vv_dsp_real start, vv_dsp_real end, size_t num, vv_dsp_real* out) {
    if (num == 0) return;
    if (num == 1) {
        out[0] = start;
        return;
    }

    vv_dsp_real step = (end - start) / (vv_dsp_real)(num - 1);
    for (size_t i = 0; i < num; i++) {
        out[i] = start + step * (vv_dsp_real)i;
    }
}

/**
 * Find the index where the value should be inserted to keep the array sorted
 */
static size_t searchsorted(const vv_dsp_real* array, size_t n, vv_dsp_real value) {
    size_t left = 0, right = n;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (array[mid] < value) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

// --------------- Mel Filterbank Generation ---------------

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
) {
    // Input validation
    if (!out_filterbank_weights || !out_num_filters || !out_filter_len) {
        return VV_DSP_ERROR_NULL_POINTER;
    }
    if (n_fft == 0 || n_mels == 0 || sample_rate <= 0.0f || fmin < 0.0f || fmax <= fmin) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    if (fmax > sample_rate / 2.0f) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    // For now, only implement HTK variant (Slaney can be added later)
    if (variant != VV_DSP_MEL_VARIANT_HTK) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    size_t n_fft_bins = n_fft / 2 + 1;  // Number of positive frequency bins

    // Check if n_mels is reasonable
    if (n_mels >= n_fft_bins) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }

    // Allocate filterbank weights matrix (flattened: n_mels * n_fft_bins)
    vv_dsp_real* filterbank = (vv_dsp_real*)calloc(n_mels * n_fft_bins, sizeof(vv_dsp_real));
    if (!filterbank) {
        return VV_DSP_ERROR_INTERNAL;
    }

    // Convert fmin and fmax to Mel scale
    vv_dsp_real mel_min = vv_dsp_hz_to_mel(fmin);
    vv_dsp_real mel_max = vv_dsp_hz_to_mel(fmax);

    // Create n_mels+2 equally spaced points in Mel scale (include fmin and fmax)
    size_t n_mel_points = n_mels + 2;
    vv_dsp_real* mel_points = (vv_dsp_real*)malloc(n_mel_points * sizeof(vv_dsp_real));
    if (!mel_points) {
        free(filterbank);
        return VV_DSP_ERROR_INTERNAL;
    }

    linspace(mel_min, mel_max, n_mel_points, mel_points);

    // Convert Mel points back to Hz
    vv_dsp_real* hz_points = (vv_dsp_real*)malloc(n_mel_points * sizeof(vv_dsp_real));
    if (!hz_points) {
        free(filterbank);
        free(mel_points);
        return VV_DSP_ERROR_INTERNAL;
    }

    for (size_t i = 0; i < n_mel_points; i++) {
        hz_points[i] = vv_dsp_mel_to_hz(mel_points[i]);
    }

    // Create frequency bins for FFT
    vv_dsp_real* fft_freqs = (vv_dsp_real*)malloc(n_fft_bins * sizeof(vv_dsp_real));
    if (!fft_freqs) {
        free(filterbank);
        free(mel_points);
        free(hz_points);
        return VV_DSP_ERROR_INTERNAL;
    }

    for (size_t i = 0; i < n_fft_bins; i++) {
        fft_freqs[i] = (vv_dsp_real)i * sample_rate / (vv_dsp_real)n_fft;
    }

    // Build triangular filters
    for (size_t m = 0; m < n_mels; m++) {
        vv_dsp_real left = hz_points[m];      // Left edge of triangle
        vv_dsp_real center = hz_points[m + 1]; // Center (peak) of triangle
        vv_dsp_real right = hz_points[m + 2];  // Right edge of triangle

        // Find FFT bin indices for triangle boundaries
        size_t left_idx = searchsorted(fft_freqs, n_fft_bins, left);
        size_t center_idx = searchsorted(fft_freqs, n_fft_bins, center);
        size_t right_idx = searchsorted(fft_freqs, n_fft_bins, right);

        // Build triangular filter response
        for (size_t k = left_idx; k < center_idx && k < n_fft_bins; k++) {
            // Rising edge of triangle
            vv_dsp_real weight = (fft_freqs[k] - left) / (center - left);
            filterbank[m * n_fft_bins + k] = weight;
        }

        for (size_t k = center_idx; k < right_idx && k < n_fft_bins; k++) {
            // Falling edge of triangle
            vv_dsp_real weight = (right - fft_freqs[k]) / (right - center);
            filterbank[m * n_fft_bins + k] = weight;
        }

        // Normalize the filter so that the peak has magnitude 1
        // (This is optional but commonly done)
        vv_dsp_real sum = 0.0f;
        for (size_t k = 0; k < n_fft_bins; k++) {
            sum += filterbank[m * n_fft_bins + k];
        }
        if (sum > 0.0f) {
            for (size_t k = 0; k < n_fft_bins; k++) {
                filterbank[m * n_fft_bins + k] /= sum;
            }
        }
    }

    // Clean up temporary arrays
    free(mel_points);
    free(hz_points);
    free(fft_freqs);

    // Set output parameters
    *out_filterbank_weights = filterbank;
    *out_num_filters = n_mels;
    *out_filter_len = n_fft_bins;

    return VV_DSP_OK;
}

void vv_dsp_mel_filterbank_free(vv_dsp_real* filterbank_weights, size_t n_mels) {
    (void)n_mels;  // Unused parameter
    if (filterbank_weights) {
        free(filterbank_weights);
    }
}

// --------------- Log-Mel Spectrogram Computation ---------------

VV_DSP_NODISCARD vv_dsp_status vv_dsp_compute_log_mel_spectrogram(
    const vv_dsp_real* power_spectrogram,
    size_t num_frames,
    size_t n_fft_bins,
    const vv_dsp_real* filterbank_weights,
    size_t n_mels,
    vv_dsp_real log_epsilon,
    vv_dsp_real* out_log_mel_spectrogram
) {
    // Input validation
    if (!power_spectrogram || !filterbank_weights || !out_log_mel_spectrogram) {
        return VV_DSP_ERROR_NULL_POINTER;
    }
    if (num_frames == 0 || n_fft_bins == 0 || n_mels == 0) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    if (log_epsilon < 0.0f) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    // Process each frame
    for (size_t frame = 0; frame < num_frames; frame++) {
        const vv_dsp_real* frame_power = &power_spectrogram[frame * n_fft_bins];
        vv_dsp_real* frame_log_mel = &out_log_mel_spectrogram[frame * n_mels];

        // Apply each Mel filter
        for (size_t m = 0; m < n_mels; m++) {
            vv_dsp_real mel_energy = 0.0f;
            const vv_dsp_real* filter_weights = &filterbank_weights[m * n_fft_bins];

            // Weighted sum (matrix multiplication)
            for (size_t k = 0; k < n_fft_bins; k++) {
                mel_energy += frame_power[k] * filter_weights[k];
            }

            // Apply logarithm with epsilon to avoid log(0)
            frame_log_mel[m] = logf(mel_energy + log_epsilon);
        }
    }

    return VV_DSP_OK;
}

// --------------- MFCC Computation ---------------

VV_DSP_NODISCARD vv_dsp_status vv_dsp_mfcc(
    const vv_dsp_real* log_mel_spectrogram,
    size_t num_frames,
    size_t n_mels,
    size_t num_mfcc_coeffs,
    vv_dsp_dct_type dct_type,
    vv_dsp_real lifter_coeff,
    vv_dsp_real* out_mfcc_coeffs
) {
    // Input validation
    if (!log_mel_spectrogram || !out_mfcc_coeffs) {
        return VV_DSP_ERROR_NULL_POINTER;
    }
    if (num_frames == 0 || n_mels == 0 || num_mfcc_coeffs == 0) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    if (num_mfcc_coeffs > n_mels) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    if (dct_type != VV_DSP_DCT_II) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }
    if (lifter_coeff < 0.0f) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    // Allocate temporary buffer for DCT output
    vv_dsp_real* dct_output = (vv_dsp_real*)malloc(n_mels * sizeof(vv_dsp_real));
    if (!dct_output) {
        return VV_DSP_ERROR_INTERNAL;
    }

    // Process each frame
    for (size_t frame = 0; frame < num_frames; frame++) {
        const vv_dsp_real* frame_log_mel = &log_mel_spectrogram[frame * n_mels];
        vv_dsp_real* frame_mfcc = &out_mfcc_coeffs[frame * num_mfcc_coeffs];

        // Apply DCT-II
        vv_dsp_status status = vv_dsp_dct_forward(n_mels, dct_type, frame_log_mel, dct_output);
        if (status != VV_DSP_OK) {
            free(dct_output);
            return status;
        }

        // Copy only the requested number of coefficients
        for (size_t i = 0; i < num_mfcc_coeffs; i++) {
            frame_mfcc[i] = dct_output[i];
        }

        // Apply liftering if requested
        if (lifter_coeff > 0.0f) {
            for (size_t i = 1; i < num_mfcc_coeffs; i++) {  // Skip c[0]
                vv_dsp_real lifter_factor = 1.0f + (lifter_coeff / 2.0f) * sinf((vv_dsp_real)M_PI * (vv_dsp_real)i / lifter_coeff);
                frame_mfcc[i] *= lifter_factor;
            }
        }
    }

    free(dct_output);
    return VV_DSP_OK;
}

// --------------- MFCC Context Management (Placeholder for Subtask 20.5) ---------------

// MFCC plan structure (will be implemented in Subtask 20.5)
struct vv_dsp_mfcc_plan {
    size_t n_fft;
    size_t n_mels;
    size_t num_mfcc_coeffs;
    size_t n_fft_bins;
    vv_dsp_real sample_rate;
    vv_dsp_real fmin;
    vv_dsp_real fmax;
    vv_dsp_mel_variant variant;
    vv_dsp_dct_type dct_type;
    vv_dsp_real lifter_coeff;
    vv_dsp_real log_epsilon;

    // Pre-computed resources
    vv_dsp_real* filterbank_weights;
    vv_dsp_real* temp_log_mel;   // Temporary buffer for log-mel spectrogram
    vv_dsp_real* temp_dct;       // Temporary buffer for DCT output
};

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
) {
    // This will be fully implemented in Subtask 20.5
    // For now, provide a basic implementation

    if (!out_plan) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

    // Input validation
    if (n_fft == 0 || n_mels == 0 || num_mfcc_coeffs == 0 || sample_rate <= 0.0f) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }
    if (num_mfcc_coeffs > n_mels || fmin < 0.0f || fmax <= fmin || fmax > sample_rate / 2.0f) {
        return VV_DSP_ERROR_OUT_OF_RANGE;
    }

    // Allocate plan structure
    vv_dsp_mfcc_plan* plan = (vv_dsp_mfcc_plan*)malloc(sizeof(vv_dsp_mfcc_plan));
    if (!plan) {
        return VV_DSP_ERROR_INTERNAL;
    }

    // Initialize plan parameters
    plan->n_fft = n_fft;
    plan->n_mels = n_mels;
    plan->num_mfcc_coeffs = num_mfcc_coeffs;
    plan->n_fft_bins = n_fft / 2 + 1;
    plan->sample_rate = sample_rate;
    plan->fmin = fmin;
    plan->fmax = fmax;
    plan->variant = variant;
    plan->dct_type = dct_type;
    plan->lifter_coeff = lifter_coeff;
    plan->log_epsilon = log_epsilon;

    // Create filterbank
    size_t num_filters, filter_len;
    vv_dsp_status status = vv_dsp_mel_filterbank_create(
        n_fft, n_mels, sample_rate, fmin, fmax, variant,
        &plan->filterbank_weights, &num_filters, &filter_len
    );

    if (status != VV_DSP_OK) {
        free(plan);
        return status;
    }

    // Allocate temporary buffers (will be used in vv_dsp_mfcc_process)
    plan->temp_log_mel = (vv_dsp_real*)malloc(n_mels * sizeof(vv_dsp_real));
    plan->temp_dct = (vv_dsp_real*)malloc(n_mels * sizeof(vv_dsp_real));

    if (!plan->temp_log_mel || !plan->temp_dct) {
        vv_dsp_mel_filterbank_free(plan->filterbank_weights, n_mels);
        free(plan->temp_log_mel);
        free(plan->temp_dct);
        free(plan);
        return VV_DSP_ERROR_INTERNAL;
    }

    *out_plan = plan;
    return VV_DSP_OK;
}

VV_DSP_NODISCARD vv_dsp_status vv_dsp_mfcc_process(
    const vv_dsp_mfcc_plan* plan,
    const vv_dsp_real* power_spectrogram,
    size_t num_frames,
    vv_dsp_real* out_mfcc_coeffs
) {
    if (!plan || !power_spectrogram || !out_mfcc_coeffs) {
        return VV_DSP_ERROR_NULL_POINTER;
    }
    if (num_frames == 0) {
        return VV_DSP_ERROR_INVALID_SIZE;
    }

    // This will be optimized in Subtask 20.5
    // For now, use the existing functions

    // Allocate temporary log-mel spectrogram for all frames
    vv_dsp_real* temp_log_mel_all = (vv_dsp_real*)malloc(num_frames * plan->n_mels * sizeof(vv_dsp_real));
    if (!temp_log_mel_all) {
        return VV_DSP_ERROR_INTERNAL;
    }

    // Compute log-mel spectrogram
    vv_dsp_status status = vv_dsp_compute_log_mel_spectrogram(
        power_spectrogram, num_frames, plan->n_fft_bins,
        plan->filterbank_weights, plan->n_mels, plan->log_epsilon,
        temp_log_mel_all
    );

    if (status != VV_DSP_OK) {
        free(temp_log_mel_all);
        return status;
    }

    // Compute MFCC
    status = vv_dsp_mfcc(
        temp_log_mel_all, num_frames, plan->n_mels, plan->num_mfcc_coeffs,
        plan->dct_type, plan->lifter_coeff, out_mfcc_coeffs
    );

    free(temp_log_mel_all);
    return status;
}

vv_dsp_status vv_dsp_mfcc_destroy(vv_dsp_mfcc_plan* plan) {
    if (!plan) {
        return VV_DSP_ERROR_NULL_POINTER;
    }

    vv_dsp_mel_filterbank_free(plan->filterbank_weights, plan->n_mels);
    free(plan->temp_log_mel);
    free(plan->temp_dct);
    free(plan);

    return VV_DSP_OK;
}
