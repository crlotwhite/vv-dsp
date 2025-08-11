/**
 * @file bench_resample_fixed.c
 * @brief Resampling benchmarks (corrected API usage)
 */

#include "vv_dsp/vv_dsp.h"
#include "bench_framework.h"
#include "bench_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RESAMPLE_SIGNAL_LEN 16384
#define RESAMPLE_NUM_ITERATIONS 50

static vv_dsp_real input_signal[RESAMPLE_SIGNAL_LEN];

/* Generate test signal */
static void generate_test_signal(void) {
    size_t i;
    for (i = 0; i < RESAMPLE_SIGNAL_LEN; i++) {
        vv_dsp_real t = (vv_dsp_real)i / RESAMPLE_SIGNAL_LEN;
        input_signal[i] = (vv_dsp_real)(0.3 * sin(2.0 * M_PI * 440.0 * t) +    /* 440Hz */
                                       0.2 * sin(2.0 * M_PI * 1000.0 * t) +   /* 1kHz */
                                       0.2 * sin(2.0 * M_PI * 2000.0 * t) +   /* 2kHz */
                                       0.1 * sin(2.0 * M_PI * 4000.0 * t) +   /* 4kHz */
                                       0.1 * sin(2.0 * M_PI * 8000.0 * t));   /* 8kHz */
    }
}

/* Benchmark resampling with different ratios */
static void benchmark_resample_ratio(vv_bench_suite* suite) {
    generate_test_signal();

    /* Test different resampling ratios */
    const struct {
        unsigned int num, den;
        const char* name;
    } ratios[] = {
        {1, 2, "Downsample_2x"},
        {2, 1, "Upsample_2x"},
        {3, 2, "Resample_3_2"},
        {4, 3, "Resample_4_3"},
        {22050, 48000, "CD_to_48k"}
    };
    const size_t num_ratios = sizeof(ratios) / sizeof(ratios[0]);

    size_t ratio_idx;
    for (ratio_idx = 0; ratio_idx < num_ratios; ratio_idx++) {
        unsigned int ratio_num = ratios[ratio_idx].num;
        unsigned int ratio_den = ratios[ratio_idx].den;
        const char* ratio_name = ratios[ratio_idx].name;

        /* Create resampler */
        vv_dsp_resampler* resampler = vv_dsp_resampler_create(ratio_num, ratio_den);
        if (!resampler) {
            printf("Failed to create resampler for ratio %u/%u\n", ratio_num, ratio_den);
            continue;
        }

        /* Calculate output buffer size */
        size_t max_output_len = (RESAMPLE_SIGNAL_LEN * ratio_num) / ratio_den + 100; /* Extra margin */
        vv_dsp_real* output_buffer = (vv_dsp_real*)malloc(max_output_len * sizeof(vv_dsp_real));
        if (!output_buffer) {
            vv_dsp_resampler_destroy(resampler);
            continue;
        }

        /* Benchmark resampling */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < RESAMPLE_NUM_ITERATIONS; iter++) {
            size_t output_len = 0;
            int status = vv_dsp_resampler_process_real(resampler,
                                                      input_signal, RESAMPLE_SIGNAL_LEN,
                                                      output_buffer, max_output_len,
                                                      &output_len);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_input_samples = (double)(RESAMPLE_SIGNAL_LEN * iter);
        double input_samples_per_second = total_input_samples / elapsed;

        /* Create and add benchmark result */
        char name[64];
        snprintf(name, sizeof(name), "Resample_%s", ratio_name);
        vv_bench_add_result(suite, name, elapsed, input_samples_per_second, 0.0, iter);

        free(output_buffer);
        vv_dsp_resampler_destroy(resampler);
    }
}

/* Benchmark resampling quality vs speed */
static void benchmark_resample_quality_vs_speed(vv_bench_suite* suite) {
    generate_test_signal();

    /* Test different quality settings */
    const struct {
        int use_sinc;
        unsigned int taps;
        const char* name;
    } qualities[] = {
        {0, 0, "Linear"},
        {1, 16, "Sinc_16_taps"},
        {1, 32, "Sinc_32_taps"},
        {1, 64, "Sinc_64_taps"}
    };
    const size_t num_qualities = sizeof(qualities) / sizeof(qualities[0]);

    /* Fixed ratio for this test */
    const unsigned int ratio_num = 3, ratio_den = 2;

    size_t quality_idx;
    for (quality_idx = 0; quality_idx < num_qualities; quality_idx++) {
        int use_sinc = qualities[quality_idx].use_sinc;
        unsigned int taps = qualities[quality_idx].taps;
        const char* quality_name = qualities[quality_idx].name;

        /* Create resampler */
        vv_dsp_resampler* resampler = vv_dsp_resampler_create(ratio_num, ratio_den);
        if (!resampler) continue;

        /* Set quality */
        if (vv_dsp_resampler_set_quality(resampler, use_sinc, taps) != VV_DSP_OK) {
            vv_dsp_resampler_destroy(resampler);
            continue;
        }

        /* Calculate output buffer size */
        size_t max_output_len = (RESAMPLE_SIGNAL_LEN * ratio_num) / ratio_den + 100;
        vv_dsp_real* output_buffer = (vv_dsp_real*)malloc(max_output_len * sizeof(vv_dsp_real));
        if (!output_buffer) {
            vv_dsp_resampler_destroy(resampler);
            continue;
        }

        /* Benchmark resampling */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        for (iter = 0; iter < RESAMPLE_NUM_ITERATIONS; iter++) {
            size_t output_len = 0;
            int status = vv_dsp_resampler_process_real(resampler,
                                                      input_signal, RESAMPLE_SIGNAL_LEN,
                                                      output_buffer, max_output_len,
                                                      &output_len);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double total_input_samples = (double)(RESAMPLE_SIGNAL_LEN * iter);
        double input_samples_per_second = total_input_samples / elapsed;

        /* Create and add benchmark result */
        char name[64];
        snprintf(name, sizeof(name), "ResampleQuality_%s", quality_name);
        vv_bench_add_result(suite, name, elapsed, input_samples_per_second, 0.0, iter);

        free(output_buffer);
        vv_dsp_resampler_destroy(resampler);
    }
}

/* Benchmark short buffer resampling (streaming) */
static void benchmark_short_buffer_resampling(vv_bench_suite* suite) {
    generate_test_signal();

    /* Test different buffer sizes for streaming */
    const size_t buffer_sizes[] = {64, 128, 256, 512, 1024};
    const size_t num_buffer_sizes = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);

    /* Fixed ratio for this test */
    const unsigned int ratio_num = 2, ratio_den = 1; /* 2x upsample */

    size_t buffer_idx;
    for (buffer_idx = 0; buffer_idx < num_buffer_sizes; buffer_idx++) {
        size_t buffer_size = buffer_sizes[buffer_idx];

        /* Create resampler */
        vv_dsp_resampler* resampler = vv_dsp_resampler_create(ratio_num, ratio_den);
        if (!resampler) continue;

        /* Set good quality */
        vv_dsp_resampler_set_quality(resampler, 1, 32);

        /* Calculate output buffer size */
        size_t max_output_len = buffer_size * 3; /* Extra margin for streaming */
        vv_dsp_real* output_buffer = (vv_dsp_real*)malloc(max_output_len * sizeof(vv_dsp_real));
        if (!output_buffer) {
            vv_dsp_resampler_destroy(resampler);
            continue;
        }

        /* Benchmark short buffer resampling */
        vv_bench_time start = vv_bench_get_time();

        size_t iter;
        size_t total_processed = 0;
        for (iter = 0; iter < RESAMPLE_NUM_ITERATIONS; iter++) {
            /* Process signal in chunks */
            size_t pos;
            for (pos = 0; pos + buffer_size <= RESAMPLE_SIGNAL_LEN; pos += buffer_size) {
                size_t output_len = 0;
                int status = vv_dsp_resampler_process_real(resampler,
                                                          input_signal + pos, buffer_size,
                                                          output_buffer, max_output_len,
                                                          &output_len);
                if (status != VV_DSP_OK) goto short_buffer_done;
                total_processed += buffer_size;
            }
        }

short_buffer_done:
        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double input_samples_per_second = (double)total_processed / elapsed;

        /* Create and add benchmark result */
        char name[64];
        snprintf(name, sizeof(name), "ResampleStreaming_%zu_samples", buffer_size);
        vv_bench_add_result(suite, name, elapsed, input_samples_per_second, 0.0, iter);

        free(output_buffer);
        vv_dsp_resampler_destroy(resampler);
    }
}

/* Main resample benchmark function */
void run_resample_benchmarks(vv_bench_suite* suite) {
    printf("Running resample benchmarks...\n");

    benchmark_resample_ratio(suite);
    benchmark_resample_quality_vs_speed(suite);
    benchmark_short_buffer_resampling(suite);

    printf("Resample benchmarks completed.\n");
}
