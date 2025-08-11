/**
 * @file bench_resample.c
 * @brief Audio resampling benchmarks
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
#define RESAMPLE_INPUT_LEN 48000     /* 1 second at 48kHz */
#define RESAMPLE_OUTPUT_LEN 96000    /* Up to 2x upsampling */
#define RESAMPLE_NUM_ITERATIONS 10   /* Number of benchmark iterations */

/* Global test data */
static vv_dsp_real input_signal[RESAMPLE_INPUT_LEN];
static vv_dsp_real output_signal[RESAMPLE_OUTPUT_LEN];

static void generate_test_signal(vv_dsp_real* signal, size_t length, vv_dsp_real sample_rate) {
    size_t i;

    /* Generate a multi-tone test signal suitable for resampling tests */
    for (i = 0; i < length; i++) {
        vv_dsp_real t = (vv_dsp_real)i / sample_rate;

        /* Mix of frequencies well below Nyquist to avoid aliasing */
        signal[i] = (vv_dsp_real)(0.3 * sin(2.0 * M_PI * 440.0 * t) +    /* 440Hz */
                                  0.2 * sin(2.0 * M_PI * 1000.0 * t) +   /* 1kHz */
                                  0.2 * sin(2.0 * M_PI * 2000.0 * t) +   /* 2kHz */
                                  0.1 * sin(2.0 * M_PI * 4000.0 * t) +   /* 4kHz */
                                  0.1 * sin(2.0 * M_PI * 8000.0 * t));   /* 8kHz */
    }
}

static void benchmark_resample_ratio(vv_bench_suite* suite,
                                     const char* name,
                                     vv_dsp_real input_rate,
                                     vv_dsp_real output_rate) {
    /* Calculate expected output length */
    size_t input_len = RESAMPLE_INPUT_LEN;
    size_t expected_output_len = (size_t)((double)input_len * (double)output_rate / (double)input_rate);

    if (expected_output_len > RESAMPLE_OUTPUT_LEN) {
        fprintf(stderr, "Warning: Expected output length %zu exceeds buffer size for %s\n",
                expected_output_len, name);
        return;
    }

    /* Generate test signal */
    generate_test_signal(input_signal, input_len, input_rate);

    /* Benchmark resampling */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    size_t actual_output_len = 0;

    for (iter = 0; iter < RESAMPLE_NUM_ITERATIONS; iter++) {
        vv_dsp_status status = vv_dsp_resample(input_signal, input_len,
                                               input_rate, output_rate,
                                               output_signal, RESAMPLE_OUTPUT_LEN,
                                               &actual_output_len);
        if (status != VV_DSP_OK) {
            fprintf(stderr, "Resampling failed for %s\n", name);
            return;
        }
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_input_samples = (double)(input_len * RESAMPLE_NUM_ITERATIONS);
    double samples_per_second = total_input_samples / elapsed;
    double input_audio_duration = (double)input_len / (double)input_rate;
    double total_audio_duration = input_audio_duration * (double)RESAMPLE_NUM_ITERATIONS;
    double rtf = elapsed / total_audio_duration;

    vv_bench_add_result(suite, name, elapsed, samples_per_second, rtf, RESAMPLE_NUM_ITERATIONS);
}

static void benchmark_common_resample_rates(vv_bench_suite* suite) {
    /* Test common resampling scenarios in audio processing */

    /* Upsampling */
    benchmark_resample_ratio(suite, "Resample_16k_to_48k", 16000.0f, 48000.0f);
    benchmark_resample_ratio(suite, "Resample_22k_to_48k", 22050.0f, 48000.0f);
    benchmark_resample_ratio(suite, "Resample_44k_to_48k", 44100.0f, 48000.0f);

    /* Downsampling */
    benchmark_resample_ratio(suite, "Resample_48k_to_16k", 48000.0f, 16000.0f);
    benchmark_resample_ratio(suite, "Resample_48k_to_22k", 48000.0f, 22050.0f);
    benchmark_resample_ratio(suite, "Resample_48k_to_44k", 48000.0f, 44100.0f);

    /* Integer ratios */
    benchmark_resample_ratio(suite, "Resample_48k_to_96k", 48000.0f, 96000.0f);  /* 2x */
    benchmark_resample_ratio(suite, "Resample_48k_to_24k", 48000.0f, 24000.0f);  /* 1/2 */
    benchmark_resample_ratio(suite, "Resample_48k_to_32k", 48000.0f, 32000.0f);  /* 2/3 */
}

static void benchmark_resample_quality_vs_speed(vv_bench_suite* suite) {
    /* If the library supports different quality settings, test them here */
    /* This is a placeholder for future quality parameter support */

    const vv_dsp_real input_rate = 48000.0f;
    const vv_dsp_real output_rate = 16000.0f;  /* Common downsampling case */
    size_t input_len = RESAMPLE_INPUT_LEN;

    /* Generate test signal */
    generate_test_signal(input_signal, input_len, input_rate);

    /* For now, just benchmark the default quality */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    size_t actual_output_len = 0;

    for (iter = 0; iter < RESAMPLE_NUM_ITERATIONS; iter++) {
        vv_dsp_status status = vv_dsp_resample(input_signal, input_len,
                                               input_rate, output_rate,
                                               output_signal, RESAMPLE_OUTPUT_LEN,
                                               &actual_output_len);
        if (status != VV_DSP_OK) break;
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_input_samples = (double)(input_len * RESAMPLE_NUM_ITERATIONS);
    double samples_per_second = total_input_samples / elapsed;
    double input_audio_duration = (double)input_len / (double)input_rate;
    double total_audio_duration = input_audio_duration * (double)RESAMPLE_NUM_ITERATIONS;
    double rtf = elapsed / total_audio_duration;

    vv_bench_add_result(suite, "Resample_default_quality", elapsed, samples_per_second, rtf, RESAMPLE_NUM_ITERATIONS);
}

static void benchmark_short_buffer_resampling(vv_bench_suite* suite) {
    /* Test resampling with smaller buffers (real-time streaming scenario) */
    const size_t short_buffer_sizes[] = {64, 128, 256, 512, 1024};
    const size_t num_sizes = sizeof(short_buffer_sizes) / sizeof(short_buffer_sizes[0]);
    const vv_dsp_real input_rate = 48000.0f;
    const vv_dsp_real output_rate = 16000.0f;

    size_t size_idx;
    for (size_idx = 0; size_idx < num_sizes; size_idx++) {
        size_t buffer_size = short_buffer_sizes[size_idx];
        size_t expected_output_size = (size_t)((double)buffer_size * (double)output_rate / (double)input_rate) + 1;

        /* Generate small test signal */
        generate_test_signal(input_signal, buffer_size, input_rate);

        /* Benchmark many small buffer operations */
        const size_t short_iterations = 1000;

        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < short_iterations; iter++) {
            size_t actual_output_len = 0;
            vv_dsp_status status = vv_dsp_resample(input_signal, buffer_size,
                                                   input_rate, output_rate,
                                                   output_signal, expected_output_size * 2,
                                                   &actual_output_len);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_input_samples = (double)(buffer_size * short_iterations);
        double samples_per_second = total_input_samples / elapsed;
        double input_audio_duration = (double)(buffer_size * short_iterations) / (double)input_rate;
        double rtf = elapsed / input_audio_duration;

        /* Create benchmark name */
        char bench_name[64];
        snprintf(bench_name, sizeof(bench_name), "Resample_short_buf_%zu", buffer_size);

        vv_bench_add_result(suite, bench_name, elapsed, samples_per_second, rtf, short_iterations);
    }
}

void run_resample_benchmarks(vv_bench_suite* suite) {
    if (!suite) return;

    /* Run resampling benchmarks */
    benchmark_common_resample_rates(suite);
    benchmark_resample_quality_vs_speed(suite);
    benchmark_short_buffer_resampling(suite);
}
