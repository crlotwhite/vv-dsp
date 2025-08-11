#include "vv_dsp/features/mel.h"
#include "vv_dsp/spectral/stft.h"
#include "vv_dsp/vv_dsp_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --sample-rate SR    Sample rate in Hz (default: 16000)\n");
    printf("  --n-fft N           FFT size (default: 512)\n");
    printf("  --hop-length H      Hop length in samples (default: 256)\n");
    printf("  --n-mels M          Number of Mel filters (default: 26)\n");
    printf("  --n-mfcc C          Number of MFCC coefficients (default: 13)\n");
    printf("  --fmin F            Minimum frequency in Hz (default: 0)\n");
    printf("  --fmax F            Maximum frequency in Hz (default: SR/2)\n");
    printf("  --lifter L          Liftering coefficient (default: 22)\n");
    printf("  --input FILE        Input signal file (default: generate test signal)\n");
    printf("  --output FILE       Output MFCC file (default: stdout)\n");
    printf("  --help              Show this help\n");
}

int main(int argc, char* argv[]) {
    // Default parameters
    vv_dsp_real sample_rate = 16000.0f;
    size_t n_fft = 512;
    size_t hop_length = 256;
    size_t n_mels = 26;
    size_t n_mfcc = 13;
    vv_dsp_real fmin = 0.0f;
    vv_dsp_real fmax = 0.0f; // Will be set to sample_rate/2 later
    vv_dsp_real lifter = 22.0f;
    const char* output_file = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--sample-rate") == 0 && i + 1 < argc) {
            sample_rate = (vv_dsp_real)atof(argv[++i]);
        } else if (strcmp(argv[i], "--n-fft") == 0 && i + 1 < argc) {
            n_fft = (size_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--hop-length") == 0 && i + 1 < argc) {
            hop_length = (size_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--n-mels") == 0 && i + 1 < argc) {
            n_mels = (size_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--n-mfcc") == 0 && i + 1 < argc) {
            n_mfcc = (size_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fmin") == 0 && i + 1 < argc) {
            fmin = (vv_dsp_real)atof(argv[++i]);
        } else if (strcmp(argv[i], "--fmax") == 0 && i + 1 < argc) {
            fmax = (vv_dsp_real)atof(argv[++i]);
        } else if (strcmp(argv[i], "--lifter") == 0 && i + 1 < argc) {
            lifter = (vv_dsp_real)atof(argv[++i]);
        } else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    // Set default fmax if not specified
    if (fmax <= 0.0f) {
        fmax = sample_rate / 2.0f;
    }

    // Input validation
    if (n_mfcc > n_mels) {
        fprintf(stderr, "Error: n_mfcc (%zu) cannot be larger than n_mels (%zu)\n", n_mfcc, n_mels);
        return 1;
    }
    if (fmin >= fmax) {
        fprintf(stderr, "Error: fmin (%.1f) must be less than fmax (%.1f)\n", (double)fmin, (double)fmax);
        return 1;
    }

    printf("# MFCC parameters:\n");
    printf("# sample_rate: %.1f\n", (double)sample_rate);
    printf("# n_fft: %zu\n", n_fft);
    printf("# hop_length: %zu\n", hop_length);
    printf("# n_mels: %zu\n", n_mels);
    printf("# n_mfcc: %zu\n", n_mfcc);
    printf("# fmin: %.1f\n", (double)fmin);
    printf("# fmax: %.1f\n", (double)fmax);
    printf("# lifter: %.1f\n", (double)lifter);

    // Create simple test signal (sine wave)
    size_t signal_length = 1024;
    vv_dsp_real* signal = (vv_dsp_real*)malloc(signal_length * sizeof(vv_dsp_real));
    if (!signal) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    // Generate test signal (440 Hz sine wave)
    vv_dsp_real freq = 440.0f;
    for (size_t i = 0; i < signal_length; i++) {
        signal[i] = sinf(2.0f * (vv_dsp_real)M_PI * freq * (vv_dsp_real)i / sample_rate);
    }

    // Generate simple power spectrogram (synthetic for demonstration)
    size_t n_fft_bins = n_fft / 2 + 1;
    size_t num_frames = 10; // Fixed number of frames for demo

    vv_dsp_real* power_spectrogram = (vv_dsp_real*)malloc(num_frames * n_fft_bins * sizeof(vv_dsp_real));
    if (!power_spectrogram) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(signal);
        return 1;
    }

    // Fill with synthetic power spectrum (decreasing with frequency)
    for (size_t frame = 0; frame < num_frames; frame++) {
        for (size_t k = 0; k < n_fft_bins; k++) {
            power_spectrogram[frame * n_fft_bins + k] = 1.0f / (1.0f + (vv_dsp_real)k * 0.1f);
        }
    }

    printf("# Computed power spectrogram: %zu frames x %zu bins\n", num_frames, n_fft_bins);

    // Create MFCC plan
    vv_dsp_mfcc_plan* plan = NULL;
    vv_dsp_status status = vv_dsp_mfcc_init(
        n_fft, n_mels, n_mfcc, sample_rate, fmin, fmax,
        VV_DSP_MEL_VARIANT_HTK, VV_DSP_DCT_II, lifter, 1e-10f, &plan
    );

    if (status != VV_DSP_OK) {
        fprintf(stderr, "Error: Failed to create MFCC plan, status=%d\n", (int)status);
        free(power_spectrogram);
        free(signal);
        return 1;
    }

    // Compute MFCC
    vv_dsp_real* mfcc_coeffs = (vv_dsp_real*)malloc(num_frames * n_mfcc * sizeof(vv_dsp_real));
    if (!mfcc_coeffs) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        vv_dsp_mfcc_destroy(plan);
        free(power_spectrogram);
        free(signal);
        return 1;
    }

    status = vv_dsp_mfcc_process(plan, power_spectrogram, num_frames, mfcc_coeffs);
    if (status != VV_DSP_OK) {
        fprintf(stderr, "Error: MFCC computation failed, status=%d\n", (int)status);
        free(mfcc_coeffs);
        vv_dsp_mfcc_destroy(plan);
        free(power_spectrogram);
        free(signal);
        return 1;
    }

    // Output MFCC coefficients
    FILE* out_fp = stdout;
    if (output_file) {
        out_fp = fopen(output_file, "w");
        if (!out_fp) {
            fprintf(stderr, "Error: Cannot open output file %s\n", output_file);
            free(mfcc_coeffs);
            vv_dsp_mfcc_destroy(plan);
            free(power_spectrogram);
            free(signal);
            return 1;
        }
    }

    fprintf(out_fp, "# MFCC coefficients (%zu frames x %zu coeffs)\n", num_frames, n_mfcc);
    for (size_t frame = 0; frame < num_frames; frame++) {
        for (size_t coeff = 0; coeff < n_mfcc; coeff++) {
            if (coeff > 0) fprintf(out_fp, " ");
            fprintf(out_fp, "%.6f", (double)mfcc_coeffs[frame * n_mfcc + coeff]);
        }
        fprintf(out_fp, "\n");
    }

    if (output_file) {
        fclose(out_fp);
    }

    printf("# Successfully computed %zu frames of MFCC with %zu coefficients each\n", num_frames, n_mfcc);

    // Cleanup
    free(mfcc_coeffs);
    vv_dsp_mfcc_destroy(plan);
    free(power_spectrogram);
    free(signal);

    return 0;
}
