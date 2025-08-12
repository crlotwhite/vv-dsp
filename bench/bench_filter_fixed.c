/**
 * @file bench_filter_fixed.c
 * @brief Filter benchmarks (corrected API usage)
 */

#include "vv_dsp/vv_dsp.h"
#include "bench_framework.h"
#include "bench_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FILTER_SIGNAL_LEN 16384
#define FILTER_NUM_ITERATIONS 100

static vv_dsp_real test_signal[FILTER_SIGNAL_LEN];
static vv_dsp_real output_signal[FILTER_SIGNAL_LEN];

/* Generate test signal with multiple frequency components */
static void generate_test_signal(void) {
    size_t i;
    for (i = 0; i < FILTER_SIGNAL_LEN; i++) {
        vv_dsp_real t = (vv_dsp_real)i / FILTER_SIGNAL_LEN;
        test_signal[i] = (vv_dsp_real)(0.4 * sin(2.0 * M_PI * 440.0 * t) +   /* 440Hz */
                                      0.3 * sin(2.0 * M_PI * 1000.0 * t) +  /* 1kHz */
                                      0.2 * sin(2.0 * M_PI * 4000.0 * t) +  /* 4kHz */
                                      0.1 * sin(2.0 * M_PI * 8000.0 * t));  /* 8kHz */
    }
}

/* Generate simple windowed sinc lowpass FIR filter */
static void generate_lowpass_fir(vv_dsp_real* coeffs, size_t M, vv_dsp_real fc) {
    size_t i;
    const vv_dsp_real fN = 0.5f; /* Normalized Nyquist */
    fc = fc / fN; /* Normalize cutoff */

    for (i = 0; i < M; i++) {
        vv_dsp_real n = (vv_dsp_real)i - (vv_dsp_real)(M - 1) / 2.0f;
        if (n == 0.0f) {
            coeffs[i] = fc;
        } else {
            vv_dsp_real x = M_PI * n;
            coeffs[i] = (vv_dsp_real)(sin(2.0 * M_PI * fc * n) / x);
        }
        /* Apply Hamming window */
        coeffs[i] *= (vv_dsp_real)(0.54 - 0.46 * cos(2.0 * M_PI * i / M));
    }
}

/* FIR time-domain benchmark using the correct API */
static void benchmark_fir_time_domain(vv_bench_suite* suite) {
    generate_test_signal();

    /* Test different filter lengths */
    const size_t filter_lens[] = {16, 32, 64, 128};
    const size_t num_filter_lens = sizeof(filter_lens) / sizeof(filter_lens[0]);

    size_t filter_idx;
    for (filter_idx = 0; filter_idx < num_filter_lens; filter_idx++) {
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

        /* Benchmark time-domain filtering */
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
        double total_samples = (double)(FILTER_SIGNAL_LEN * iter);
        double samples_per_second = total_samples / elapsed;

        /* Create benchmark result */
        vv_bench_result result;
        snprintf(result.name, sizeof(result.name), "FIR_TimeDomain_%zu_taps", filter_len);
        result.elapsed_seconds = elapsed;
        result.samples_per_second = samples_per_second;
        result.real_time_factor = 0.0;
        result.iterations = iter;
        result.valid = 1;
        vv_bench_add_result(suite, result.name, result.elapsed_seconds,
                          result.samples_per_second, result.real_time_factor, result.iterations);

        vv_dsp_fir_state_free(&fir_state);
        free(filter_coeffs);
    }
}

/* FIR FFT-domain benchmark using the correct API */
static void benchmark_fir_fft_domain(vv_bench_suite* suite) {
    generate_test_signal();

    /* Test different filter lengths */
    const size_t filter_lens[] = {16, 32, 64, 128};
    const size_t num_filter_lens = sizeof(filter_lens) / sizeof(filter_lens[0]);

    size_t filter_idx;
    for (filter_idx = 0; filter_idx < num_filter_lens; filter_idx++) {
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

        /* Benchmark FFT-domain filtering */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < FILTER_NUM_ITERATIONS; iter++) {
            vv_dsp_status status = vv_dsp_fir_apply_fft(&fir_state, filter_coeffs,
                                                       test_signal, output_signal, FILTER_SIGNAL_LEN);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_samples = (double)(FILTER_SIGNAL_LEN * iter);
        double samples_per_second = total_samples / elapsed;

        /* Create and add benchmark result */
        char name[64];
        snprintf(name, sizeof(name), "FIR_FFTDomain_%zu_taps", filter_len);
        vv_bench_add_result(suite, name, elapsed, samples_per_second, 0.0, iter);

        vv_dsp_fir_state_free(&fir_state);
        free(filter_coeffs);
    }
}

/* IIR filter benchmark using the correct API */
static void benchmark_iir_filters(vv_bench_suite* suite) {
    generate_test_signal();

    /* Create simple lowpass biquad filter */
    vv_dsp_biquad biquad;
    /* Simple lowpass: fc=0.1, Q=0.707 */
    vv_dsp_real b0 = 0.067455273f, b1 = 0.134910546f, b2 = 0.067455273f;
    vv_dsp_real a1 = -1.142980502f, a2 = 0.412801594f;

    if (vv_dsp_biquad_init(&biquad, b0, b1, b2, a1, a2) != VV_DSP_OK) {
        printf("Failed to initialize biquad filter\n");
        return;
    }

    /* Benchmark IIR filtering */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    for (iter = 0; iter < FILTER_NUM_ITERATIONS; iter++) {
        vv_dsp_status status = vv_dsp_iir_apply(&biquad, 1, /* single stage */
                                               test_signal, output_signal, FILTER_SIGNAL_LEN);
        if (status != VV_DSP_OK) break;
        vv_dsp_biquad_reset(&biquad); /* Reset state for consistent timing */
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_samples = (double)(FILTER_SIGNAL_LEN * iter);
    double samples_per_second = total_samples / elapsed;

    /* Create and add benchmark result */
    char name[64];
    snprintf(name, sizeof(name), "IIR_Biquad_1_stage");
    vv_bench_add_result(suite, name, elapsed, samples_per_second, 0.0, iter);
}

/* Savitzky-Golay filter benchmark using the correct API */
static void benchmark_savgol_filter(vv_bench_suite* suite) {
    generate_test_signal();

    /* Test different window lengths */
    const size_t window_lens[] = {11, 21, 31, 51};
    const size_t num_window_lens = sizeof(window_lens) / sizeof(window_lens[0]);

    size_t window_idx;
    for (window_idx = 0; window_idx < num_window_lens; window_idx++) {
        const size_t window_len = window_lens[window_idx];

        /* Benchmark Savitzky-Golay filtering */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < FILTER_NUM_ITERATIONS; iter++) {
            vv_dsp_status status = vv_dsp_savgol(test_signal, FILTER_SIGNAL_LEN,
                                                (int)window_len, 3, 0, 1.0,
                                                VV_DSP_SAVGOL_MODE_REFLECT,
                                                output_signal);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_samples = (double)(FILTER_SIGNAL_LEN * iter);
        double samples_per_second = total_samples / elapsed;

        /* Create and add benchmark result */
        char name[64];
        snprintf(name, sizeof(name), "SavGol_Window_%zu", window_len);
        vv_bench_add_result(suite, name, elapsed, samples_per_second, 0.0, iter);
    }
}

/* Main filter benchmark function */
void run_filter_benchmarks(vv_bench_suite* suite) {
    printf("Running filter benchmarks...\n");

    benchmark_fir_time_domain(suite);
    benchmark_fir_fft_domain(suite);
    benchmark_iir_filters(suite);
    benchmark_savgol_filter(suite);

    printf("Filter benchmarks completed.\n");
}
