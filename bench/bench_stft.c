/**
 * @file bench_stft.c
 * @brief STFT (Short-Time Fourier Transform) benchmarks
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
#define STFT_SIGNAL_LEN 48000      /* 1 second at 48kHz */
#define STFT_FRAME_SIZE 1024       /* Standard FFT size */
#define STFT_HOP_SIZE   256        /* 75% overlap */
#define STFT_NUM_ITERATIONS 10     /* Number of benchmark iterations */

/* Global test data to avoid stack allocation */
static vv_dsp_real test_signal[STFT_SIGNAL_LEN];
static vv_dsp_real frame_buffer[STFT_FRAME_SIZE];
static vv_dsp_real output_buffer[STFT_SIGNAL_LEN];
static vv_dsp_cpx spectrum[STFT_FRAME_SIZE];

static void generate_test_signal(vv_dsp_real* signal, size_t length) {
    size_t i;
    const vv_dsp_real sample_rate = (vv_dsp_real)48000.0;
    const vv_dsp_real freq1 = (vv_dsp_real)440.0;   /* A4 */
    const vv_dsp_real freq2 = (vv_dsp_real)880.0;   /* A5 */

    for (i = 0; i < length; i++) {
        vv_dsp_real t = (vv_dsp_real)i / sample_rate;
        signal[i] = (vv_dsp_real)(0.5 * sin(2.0 * M_PI * freq1 * t) +
                                  0.3 * sin(2.0 * M_PI * freq2 * t));
    }
}

static void benchmark_stft_processing_loop(vv_bench_suite* suite) {
    vv_dsp_stft* stft_handle = NULL;
    vv_dsp_stft_params params = {
        .fft_size = STFT_FRAME_SIZE,
        .hop_size = STFT_HOP_SIZE,
        .window = VV_DSP_STFT_WIN_HANN
    };

    /* Create STFT handle */
    vv_dsp_status status = vv_dsp_stft_create(&params, &stft_handle);
    if (status != VV_DSP_OK || !stft_handle) {
        fprintf(stderr, "Failed to create STFT handle\n");
        return;
    }

    /* Generate test signal */
    generate_test_signal(test_signal, STFT_SIGNAL_LEN);

    /* Clear output buffer */
    memset(output_buffer, 0, sizeof(output_buffer));

    /* Calculate number of frames */
    size_t num_frames = vv_dsp_get_num_frames(STFT_SIGNAL_LEN, STFT_FRAME_SIZE, STFT_HOP_SIZE, 0);

    /* Benchmark the complete analysis-synthesis loop */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    for (iter = 0; iter < STFT_NUM_ITERATIONS; iter++) {
        size_t frame_idx;

        /* Analysis phase */
        for (frame_idx = 0; frame_idx < num_frames; frame_idx++) {
            /* Fetch frame */
            status = vv_dsp_fetch_frame(test_signal, STFT_SIGNAL_LEN,
                                        frame_buffer, STFT_FRAME_SIZE,
                                        STFT_HOP_SIZE, frame_idx, 0, NULL);
            if (status != VV_DSP_OK) continue;

            /* Forward STFT */
            status = vv_dsp_stft_process(stft_handle, frame_buffer, spectrum);
            if (status != VV_DSP_OK) continue;

            /* Synthesis phase (with no-op spectral processing) */
            status = vv_dsp_stft_reconstruct(stft_handle, spectrum,
                                             output_buffer + frame_idx * STFT_HOP_SIZE, NULL);
        }
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_samples_processed = (double)(STFT_SIGNAL_LEN * STFT_NUM_ITERATIONS);
    double samples_per_second = total_samples_processed / elapsed;
    double audio_duration = (double)STFT_SIGNAL_LEN / 48000.0;  /* 1 second per iteration */
    double total_audio_duration = audio_duration * (double)STFT_NUM_ITERATIONS;
    double rtf = elapsed / total_audio_duration;

    vv_bench_add_result(suite, "STFT_processing_loop", elapsed, samples_per_second, rtf, STFT_NUM_ITERATIONS);

    /* Cleanup */
    vv_dsp_stft_destroy(stft_handle);
}

static void benchmark_stft_frame_rate(vv_bench_suite* suite) {
    vv_dsp_stft* stft_handle = NULL;
    vv_dsp_stft_params params = {
        .fft_size = STFT_FRAME_SIZE,
        .hop_size = STFT_HOP_SIZE,
        .window = VV_DSP_STFT_WIN_HANN
    };

    /* Create STFT handle */
    vv_dsp_status status = vv_dsp_stft_create(&params, &stft_handle);
    if (status != VV_DSP_OK || !stft_handle) {
        fprintf(stderr, "Failed to create STFT handle for frame rate benchmark\n");
        return;
    }

    /* Generate a single test frame */
    generate_test_signal(frame_buffer, STFT_FRAME_SIZE);

    /* Benchmark single frame processing */
    const size_t frame_iterations = 1000;

    vv_bench_time start = vv_bench_get_time();

    size_t i;
    for (i = 0; i < frame_iterations; i++) {
        status = vv_dsp_stft_process(stft_handle, frame_buffer, spectrum);
        if (status != VV_DSP_OK) break;
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate frame rate metrics */
    double frames_per_second = (double)frame_iterations / elapsed;
    double samples_per_frame = (double)STFT_HOP_SIZE;  /* Real-time processing rate */
    double max_sample_rate = frames_per_second * samples_per_frame;

    vv_bench_add_result(suite, "STFT_frame_rate", elapsed, max_sample_rate, 0.0, frame_iterations);

    /* Cleanup */
    vv_dsp_stft_destroy(stft_handle);
}

static void benchmark_stft_different_sizes(vv_bench_suite* suite) {
    /* Test different FFT sizes commonly used in audio processing */
    const size_t fft_sizes[] = {256, 512, 1024, 2048, 4096};
    const size_t num_sizes = sizeof(fft_sizes) / sizeof(fft_sizes[0]);

    size_t size_idx;
    for (size_idx = 0; size_idx < num_sizes; size_idx++) {
        size_t fft_size = fft_sizes[size_idx];
        size_t hop_size = fft_size / 4;  /* 75% overlap */

        vv_dsp_stft* stft_handle = NULL;
        vv_dsp_stft_params params = {
            .fft_size = fft_size,
            .hop_size = hop_size,
            .window = VV_DSP_STFT_WIN_HANN
        };

        vv_dsp_status status = vv_dsp_stft_create(&params, &stft_handle);
        if (status != VV_DSP_OK || !stft_handle) {
            continue;  /* Skip this size if creation fails */
        }

        /* Allocate frame buffer for this size */
        vv_dsp_real* frame = (vv_dsp_real*)malloc(fft_size * sizeof(vv_dsp_real));
        vv_dsp_cpx* spec = (vv_dsp_cpx*)malloc(fft_size * sizeof(vv_dsp_cpx));
        if (!frame || !spec) {
            free(frame);
            free(spec);
            vv_dsp_stft_destroy(stft_handle);
            continue;
        }

        /* Generate test frame */
        generate_test_signal(frame, fft_size);

        /* Benchmark this size */
        const size_t size_iterations = 200;

        vv_bench_time start = vv_bench_get_time();

        size_t i;
        for (i = 0; i < size_iterations; i++) {
            status = vv_dsp_stft_process(stft_handle, frame, spec);
            if (status != VV_DSP_OK) break;
        }

        vv_bench_time end = vv_bench_get_time();
        double elapsed = vv_bench_elapsed_seconds(start, end);

        /* Calculate metrics */
        double frames_per_second = (double)size_iterations / elapsed;
        double max_sample_rate = frames_per_second * (double)hop_size;

        /* Create benchmark name */
        char bench_name[64];
        snprintf(bench_name, sizeof(bench_name), "STFT_size_%zu", fft_size);

        vv_bench_add_result(suite, bench_name, elapsed, max_sample_rate, 0.0, size_iterations);

        /* Cleanup */
        free(frame);
        free(spec);
        vv_dsp_stft_destroy(stft_handle);
    }
}

void run_stft_benchmarks(vv_bench_suite* suite) {
    if (!suite) return;

    /* Run STFT benchmarks */
    benchmark_stft_processing_loop(suite);
    benchmark_stft_frame_rate(suite);
    benchmark_stft_different_sizes(suite);
}
