/**
 * @file bench_filter.c
 * @brief FIR and IIR filter benchmarks
 * @ingroup benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vv_dsp/vv_dsp.h"
#include "bench_framework.h"
#include "bench_timer.h"

/* MSVC compatibility: Use fixed-size arrays */
#define FILTER_SIGNAL_LEN 48000    /* 1 second at 48kHz */
#define FILTER_NUM_ITERATIONS 10   /* Number of benchmark iterations */
#define MAX_FILTER_ORDER 1023      /* Maximum filter length to test */

/* Global test data */
static vv_dsp_real test_signal[FILTER_SIGNAL_LEN];
static vv_dsp_real output_signal[FILTER_SIGNAL_LEN];
static vv_dsp_real filter_coeffs[MAX_FILTER_ORDER + 1];

static void generate_test_signal(vv_dsp_real* signal, size_t length) {
    size_t i;
    const vv_dsp_real sample_rate = (vv_dsp_real)48000.0;

    /* Generate a mix of frequencies for filter testing */
    for (i = 0; i < length; i++) {
        vv_dsp_real t = (vv_dsp_real)i / sample_rate;
        signal[i] = (vv_dsp_real)(0.4 * sin(2.0 * M_PI * 440.0 * t) +   /* 440Hz */
                                  0.3 * sin(2.0 * M_PI * 1000.0 * t) +  /* 1kHz */
                                  0.2 * sin(2.0 * M_PI * 4000.0 * t) +  /* 4kHz */
                                  0.1 * sin(2.0 * M_PI * 8000.0 * t));  /* 8kHz */
    }
}

static void generate_lowpass_fir(vv_dsp_real* coeffs, size_t length, vv_dsp_real cutoff_freq) {
    /* Simple windowed sinc lowpass filter design */
    size_t i;
    vv_dsp_real fc = cutoff_freq / (vv_dsp_real)48000.0;  /* Normalized frequency */
    int M = (int)length - 1;  /* Filter order */

    for (i = 0; i < length; i++) {
        int n = (int)i - M / 2;
        if (n == 0) {
            coeffs[i] = 2.0f * fc;
        } else {
            vv_dsp_real x = (vv_dsp_real)M_PI * (vv_dsp_real)n;
            coeffs[i] = (vv_dsp_real)(sin(2.0 * M_PI * fc * n) / x);
        }

        /* Apply Hamming window */
        coeffs[i] *= (vv_dsp_real)(0.54 - 0.46 * cos(2.0 * M_PI * i / M));
    }
}

/* FIR time-domain benchmark */
static void benchmark_fir_time_domain(benchmark_suite* suite) {
    generate_test_signal();

    /* Test different filter lengths */
    const size_t filter_lens[] = {16, 32, 64, 128};
    const size_t num_filter_lens = sizeof(filter_lens) / sizeof(filter_lens[0]);

    for (size_t filter_idx = 0; filter_idx < num_filter_lens; filter_idx++) {
        const size_t filter_len = filter_lens[filter_idx];
        vv_dsp_real* filter_coeffs = (vv_dsp_real*)malloc(filter_len * sizeof(vv_dsp_real));
        if (!filter_coeffs) continue;

        /* Generate filter coefficients */
        generate_lowpass_fir(filter_coeffs, filter_len, 0.25f);

        /* Initialize FIR state */
        vv_dsp_fir_state fir_state;
        if (vv_dsp_fir_state_init(&fir_state, filter_len) != VV_DSP_OK) {
            free(filter_coeffs);
            continue;
        }

        /* Benchmark time-domain convolution */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < FILTER_NUM_ITERATIONS; iter++) {
            vv_dsp_status status = vv_dsp_fir_apply(&fir_state, filter_coeffs,
                                                   test_signal, output_signal, FILTER_SIGNAL_LEN);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_samples = (double)(FILTER_SIGNAL_LEN * FILTER_NUM_ITERATIONS);
        double samples_per_second = total_samples / elapsed;

        /* Create benchmark result */
        benchmark_result result;
        snprintf(result.name, sizeof(result.name), "FIR_TimeDomain_%zu_taps", filter_len);
        result.elapsed_time = elapsed;
        result.samples_per_second = samples_per_second;
        result.iterations = iter;
        benchmark_suite_add_result(suite, result);

        vv_dsp_fir_state_free(&fir_state);
        free(filter_coeffs);
    }
}

static void benchmark_fir_fft_domain(vv_bench_suite* suite) {
    /* Test FFT-based convolution for long filters */
    const size_t filter_lengths[] = {255, 511, 1023};
    const size_t num_lengths = sizeof(filter_lengths) / sizeof(filter_lengths[0]);

    /* Generate test signal */
    generate_test_signal(test_signal, FILTER_SIGNAL_LEN);

    size_t len_idx;
    for (len_idx = 0; len_idx < num_lengths; len_idx++) {
        size_t filter_len = filter_lengths[len_idx];

        /* Generate filter coefficients */
        generate_lowpass_fir(filter_coeffs, filter_len, 4000.0f);

        /* Benchmark FFT-based convolution */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < FILTER_NUM_ITERATIONS; iter++) {
            vv_dsp_status status = vv_dsp_convolve_fft_real(test_signal, FILTER_SIGNAL_LEN,
                                                            filter_coeffs, filter_len,
                                                            output_signal, FILTER_SIGNAL_LEN);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_samples = (double)(FILTER_SIGNAL_LEN * FILTER_NUM_ITERATIONS);
        double samples_per_second = total_samples / elapsed;
        double audio_duration = (double)FILTER_SIGNAL_LEN / 48000.0;
        double total_audio_duration = audio_duration * (double)FILTER_NUM_ITERATIONS;
        double rtf = elapsed / total_audio_duration;

        /* Create benchmark name */
        char bench_name[64];
        snprintf(bench_name, sizeof(bench_name), "FIR_fft_domain_taps_%zu", filter_len);

        vv_bench_add_result(suite, bench_name, elapsed, samples_per_second, rtf, FILTER_NUM_ITERATIONS);
    }
}

static void benchmark_iir_filters(vv_bench_suite* suite) {
    /* Test IIR filter performance */
    /* Simple 2nd order lowpass filter coefficients (normalized to fs=48kHz, fc=4kHz) */
    const vv_dsp_real a_coeffs[] = {1.0f, -0.9428f, 0.3333f};  /* Denominator */
    const vv_dsp_real b_coeffs[] = {0.0976f, 0.1953f, 0.0976f}; /* Numerator */
    const size_t filter_order = 2;

    /* Generate test signal */
    generate_test_signal(test_signal, FILTER_SIGNAL_LEN);

    /* Benchmark IIR filtering */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    for (iter = 0; iter < FILTER_NUM_ITERATIONS; iter++) {
        vv_dsp_status status = vv_dsp_iir_filter(test_signal, FILTER_SIGNAL_LEN,
                                                 b_coeffs, a_coeffs, filter_order,
                                                 output_signal, FILTER_SIGNAL_LEN);
        if (status != VV_DSP_OK) break;
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_samples = (double)(FILTER_SIGNAL_LEN * FILTER_NUM_ITERATIONS);
    double samples_per_second = total_samples / elapsed;
    double audio_duration = (double)FILTER_SIGNAL_LEN / 48000.0;
    double total_audio_duration = audio_duration * (double)FILTER_NUM_ITERATIONS;
    double rtf = elapsed / total_audio_duration;

    vv_bench_add_result(suite, "IIR_2nd_order", elapsed, samples_per_second, rtf, FILTER_NUM_ITERATIONS);
}

static void benchmark_savgol_filter(vv_bench_suite* suite) {
    /* Test Savitzky-Golay filter performance */
    const size_t window_lengths[] = {5, 9, 15, 25, 51};
    const size_t num_lengths = sizeof(window_lengths) / sizeof(window_lengths[0]);

    /* Generate test signal */
    generate_test_signal(test_signal, FILTER_SIGNAL_LEN);

    size_t len_idx;
    for (len_idx = 0; len_idx < num_lengths; len_idx++) {
        size_t window_len = window_lengths[len_idx];

        /* Benchmark Savitzky-Golay filtering */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < FILTER_NUM_ITERATIONS; iter++) {
            vv_dsp_status status = vv_dsp_savgol_filter(test_signal, FILTER_SIGNAL_LEN,
                                                        window_len, 3, 0, 1.0f,
                                                        VV_DSP_SAVGOL_MODE_NEAREST,
                                                        output_signal);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_samples = (double)(FILTER_SIGNAL_LEN * FILTER_NUM_ITERATIONS);
        double samples_per_second = total_samples / elapsed;
        double audio_duration = (double)FILTER_SIGNAL_LEN / 48000.0;
        double total_audio_duration = audio_duration * (double)FILTER_NUM_ITERATIONS;
        double rtf = elapsed / total_audio_duration;

        /* Create benchmark name */
        char bench_name[64];
        snprintf(bench_name, sizeof(bench_name), "Savgol_window_%zu", window_len);

        vv_bench_add_result(suite, bench_name, elapsed, samples_per_second, rtf, FILTER_NUM_ITERATIONS);
    }
}

void run_filter_benchmarks(vv_bench_suite* suite) {
    if (!suite) return;

    /* Run filter benchmarks */
    benchmark_fir_time_domain(suite);
    benchmark_fir_fft_domain(suite);
    benchmark_iir_filters(suite);
    benchmark_savgol_filter(suite);
}
