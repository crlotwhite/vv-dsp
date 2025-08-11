#include "vv_dsp/features/mel.h"
#include "vv_dsp/vv_dsp_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TOLERANCE 1e-3f

static int test_mel_scale_conversions(void) {
    printf("Testing Mel scale conversions...\n");
    
    // Test specific frequencies
    vv_dsp_real hz_values[] = {0.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f};
    size_t n_tests = sizeof(hz_values) / sizeof(hz_values[0]);
    
    for (size_t i = 0; i < n_tests; i++) {
        vv_dsp_real hz = hz_values[i];
        vv_dsp_real mel = vv_dsp_hz_to_mel(hz);
        vv_dsp_real hz_back = vv_dsp_mel_to_hz(mel);
        
        printf("  Hz: %.1f -> Mel: %.3f -> Hz: %.3f\n", (double)hz, (double)mel, (double)hz_back);
        
        // Check round-trip conversion
        if (fabsf(hz - hz_back) > TOLERANCE) {
            printf("ERROR: Round-trip conversion failed for Hz=%.1f\n", (double)hz);
            return 1;
        }
    }
    
    // Test edge cases
    if (vv_dsp_hz_to_mel(-100.0f) != 0.0f) {
        printf("ERROR: Negative Hz should return 0\n");
        return 1;
    }
    
    if (vv_dsp_mel_to_hz(-100.0f) != 0.0f) {
        printf("ERROR: Negative Mel should return 0\n");
        return 1;
    }
    
    printf("Mel scale conversions PASSED\n");
    return 0;
}

static int test_mel_filterbank_creation(void) {
    printf("Testing Mel filterbank creation...\n");
    
    size_t n_fft = 512;
    size_t n_mels = 26;
    vv_dsp_real sample_rate = 16000.0f;
    vv_dsp_real fmin = 0.0f;
    vv_dsp_real fmax = 8000.0f;
    
    vv_dsp_real* filterbank_weights = NULL;
    size_t num_filters, filter_len;
    
    vv_dsp_status status = vv_dsp_mel_filterbank_create(
        n_fft, n_mels, sample_rate, fmin, fmax, VV_DSP_MEL_VARIANT_HTK,
        &filterbank_weights, &num_filters, &filter_len
    );
    
    if (status != VV_DSP_OK) {
        printf("ERROR: Failed to create Mel filterbank, status=%d\n", (int)status);
        return 1;
    }
    
    if (num_filters != n_mels) {
        printf("ERROR: Expected %zu filters, got %zu\n", n_mels, num_filters);
        vv_dsp_mel_filterbank_free(filterbank_weights, n_mels);
        return 1;
    }
    
    if (filter_len != n_fft/2 + 1) {
        printf("ERROR: Expected filter length %zu, got %zu\n", n_fft/2 + 1, filter_len);
        vv_dsp_mel_filterbank_free(filterbank_weights, n_mels);
        return 1;
    }
    
    // Check that filterbank has some non-zero values
    int has_nonzero = 0;
    for (size_t i = 0; i < num_filters * filter_len; i++) {
        if (filterbank_weights[i] > 0.0f) {
            has_nonzero = 1;
            break;
        }
    }
    
    if (!has_nonzero) {
        printf("ERROR: Filterbank contains no non-zero values\n");
        vv_dsp_mel_filterbank_free(filterbank_weights, n_mels);
        return 1;
    }
    
    printf("Created %zu Mel filters with length %zu\n", num_filters, filter_len);
    vv_dsp_mel_filterbank_free(filterbank_weights, n_mels);
    
    printf("Mel filterbank creation PASSED\n");
    return 0;
}

static int test_mfcc_basic(void) {
    printf("Testing basic MFCC computation...\n");
    
    size_t n_fft = 512;
    size_t n_mels = 26;
    size_t num_mfcc_coeffs = 13;
    vv_dsp_real sample_rate = 16000.0f;
    vv_dsp_real fmin = 0.0f;
    vv_dsp_real fmax = 8000.0f;
    vv_dsp_real log_epsilon = 1e-10f;
    
    // Create MFCC plan
    vv_dsp_mfcc_plan* plan = NULL;
    vv_dsp_status status = vv_dsp_mfcc_init(
        n_fft, n_mels, num_mfcc_coeffs, sample_rate, fmin, fmax,
        VV_DSP_MEL_VARIANT_HTK, VV_DSP_DCT_II, 22.0f, log_epsilon, &plan
    );
    
    if (status != VV_DSP_OK) {
        printf("ERROR: Failed to create MFCC plan, status=%d\n", (int)status);
        return 1;
    }
    
    // Create synthetic power spectrogram (single frame)
    size_t num_frames = 1;
    size_t n_fft_bins = n_fft / 2 + 1;
    vv_dsp_real* power_spectrogram = (vv_dsp_real*)malloc(num_frames * n_fft_bins * sizeof(vv_dsp_real));
    if (!power_spectrogram) {
        printf("ERROR: Memory allocation failed\n");
        vv_dsp_mfcc_destroy(plan);
        return 1;
    }
    
    // Fill with synthetic data (simple decreasing spectrum)
    for (size_t k = 0; k < n_fft_bins; k++) {
        power_spectrogram[k] = 1.0f / (1.0f + (vv_dsp_real)k);
    }
    
    // Compute MFCC
    vv_dsp_real* mfcc_coeffs = (vv_dsp_real*)malloc(num_frames * num_mfcc_coeffs * sizeof(vv_dsp_real));
    if (!mfcc_coeffs) {
        printf("ERROR: Memory allocation failed\n");
        free(power_spectrogram);
        vv_dsp_mfcc_destroy(plan);
        return 1;
    }
    
    status = vv_dsp_mfcc_process(plan, power_spectrogram, num_frames, mfcc_coeffs);
    if (status != VV_DSP_OK) {
        printf("ERROR: MFCC computation failed, status=%d\n", (int)status);
        free(power_spectrogram);
        free(mfcc_coeffs);
        vv_dsp_mfcc_destroy(plan);
        return 1;
    }
    
        // Check that MFCC values are finite
        for (size_t i = 0; i < num_mfcc_coeffs; i++) {
            if (!isfinite(mfcc_coeffs[i])) {
                printf("ERROR: MFCC coefficient %zu is not finite: %f\n", i, (double)mfcc_coeffs[i]);
                free(power_spectrogram);
                free(mfcc_coeffs);
                vv_dsp_mfcc_destroy(plan);
                return 1;
            }
        }
        
        printf("MFCC coefficients: ");
        for (size_t i = 0; i < num_mfcc_coeffs; i++) {
            printf("%.3f ", (double)mfcc_coeffs[i]);
        }
        printf("\n");    // Cleanup
    free(power_spectrogram);
    free(mfcc_coeffs);
    vv_dsp_mfcc_destroy(plan);
    
    printf("Basic MFCC computation PASSED\n");
    return 0;
}

int main(void) {
    printf("=== VV-DSP MFCC Tests ===\n");
    
    int errors = 0;
    
    errors += test_mel_scale_conversions();
    errors += test_mel_filterbank_creation();
    errors += test_mfcc_basic();
    
    if (errors == 0) {
        printf("\nAll MFCC tests PASSED!\n");
        return 0;
    } else {
        printf("\n%d test(s) FAILED!\n", errors);
        return 1;
    }
}
