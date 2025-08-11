/**
 * @file bench_pipeline.c
 * @brief End-to-end DSP pipeline benchmarks
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
#define PIPELINE_SIGNAL_LEN 480000    /* 10 seconds at 48kHz */
#define PIPELINE_FRAME_SIZE 1024      /* STFT frame size */
#define PIPELINE_HOP_SIZE 256         /* STFT hop size */
#define PIPELINE_NUM_ITERATIONS 3     /* Fewer iterations due to longer processing */

/* Global test data */
static vv_dsp_real input_signal[PIPELINE_SIGNAL_LEN];
static vv_dsp_real processed_signal[PIPELINE_SIGNAL_LEN];
static vv_dsp_real frame_buffer[PIPELINE_FRAME_SIZE];
static vv_dsp_real windowed_frame[PIPELINE_FRAME_SIZE];
static vv_dsp_cpx spectrum[PIPELINE_FRAME_SIZE];
static vv_dsp_real hann_window[PIPELINE_FRAME_SIZE];

static void generate_test_audio(vv_dsp_real* signal, size_t length) {
    size_t i;
    const vv_dsp_real sample_rate = 48000.0f;

    /* Generate realistic test audio with multiple components */
    for (i = 0; i < length; i++) {
        vv_dsp_real t = (vv_dsp_real)i / sample_rate;

        /* Speech-like formant frequencies */
        vv_dsp_real f1 = 800.0f + 200.0f * (vv_dsp_real)sin(2.0 * M_PI * 3.0 * t);   /* Varying formant */
        vv_dsp_real f2 = 1200.0f + 300.0f * (vv_dsp_real)sin(2.0 * M_PI * 2.0 * t);  /* Second formant */

        /* Background noise and harmonics */
        vv_dsp_real fundamental = 150.0f;  /* Base frequency */

        signal[i] = (vv_dsp_real)(
            0.4 * sin(2.0 * M_PI * fundamental * t) +              /* Fundamental */
            0.2 * sin(2.0 * M_PI * f1 * t) +                       /* First formant */
            0.15 * sin(2.0 * M_PI * f2 * t) +                      /* Second formant */
            0.1 * sin(2.0 * M_PI * 2.0 * fundamental * t) +        /* Second harmonic */
            0.05 * sin(2.0 * M_PI * 3.0 * fundamental * t) +       /* Third harmonic */
            0.02 * ((vv_dsp_real)rand() / RAND_MAX - 0.5f)         /* Noise */
        );
    }
}

static void generate_hann_window(vv_dsp_real* window, size_t length) {
    size_t i;
    for (i = 0; i < length; i++) {
        window[i] = (vv_dsp_real)(0.5 * (1.0 - cos(2.0 * M_PI * i / (length - 1))));
    }
}

static void spectral_processing_placeholder(vv_dsp_cpx* spectrum, size_t length) {
    /* Placeholder spectral processing: simple noise reduction */
    size_t i;
    for (i = 0; i < length; i++) {
        vv_dsp_real magnitude = (vv_dsp_real)sqrt(spectrum[i].re * spectrum[i].re +
                                                 spectrum[i].im * spectrum[i].im);

        /* Simple noise gate: attenuate low-magnitude components */
        if (magnitude < 0.01f) {
            spectrum[i].re *= 0.1f;
            spectrum[i].im *= 0.1f;
        }
    }
}

static void benchmark_complete_audio_pipeline(vv_bench_suite* suite) {
    /*
     * Complete pipeline: Input -> Pre-emphasis -> Framing -> Windowing ->
     * STFT -> Spectral Processing -> ISTFT -> Overlap-Add -> Output
     */

    vv_dsp_stft* stft_handle = NULL;
    vv_dsp_stft_params stft_params = {
        .fft_size = PIPELINE_FRAME_SIZE,
        .hop_size = PIPELINE_HOP_SIZE,
        .window = VV_DSP_STFT_WIN_HANN
    };

    /* Create STFT handle */
    vv_dsp_status status = vv_dsp_stft_create(&stft_params, &stft_handle);
    if (status != VV_DSP_OK || !stft_handle) {
        fprintf(stderr, "Failed to create STFT handle for pipeline benchmark\n");
        return;
    }

    /* Generate test audio and window */
    generate_test_audio(input_signal, PIPELINE_SIGNAL_LEN);
    generate_hann_window(hann_window, PIPELINE_FRAME_SIZE);

    /* Calculate number of frames */
    size_t num_frames = vv_dsp_get_num_frames(PIPELINE_SIGNAL_LEN, PIPELINE_FRAME_SIZE, PIPELINE_HOP_SIZE, 0);

    /* Clear output buffer */
    memset(processed_signal, 0, sizeof(processed_signal));

    /* Benchmark complete pipeline */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    for (iter = 0; iter < PIPELINE_NUM_ITERATIONS; iter++) {
        /* Reset output buffer for each iteration */
        memset(processed_signal, 0, PIPELINE_SIGNAL_LEN * sizeof(vv_dsp_real));

        size_t frame_idx;
        for (frame_idx = 0; frame_idx < num_frames; frame_idx++) {
            /* Step 1: Extract frame */
            status = vv_dsp_fetch_frame(input_signal, PIPELINE_SIGNAL_LEN,
                                        frame_buffer, PIPELINE_FRAME_SIZE,
                                        PIPELINE_HOP_SIZE, frame_idx, 0, NULL);
            if (status != VV_DSP_OK) continue;

            /* Step 2: Apply pre-emphasis (simple high-pass) */
            size_t j;
            for (j = PIPELINE_FRAME_SIZE - 1; j > 0; j--) {
                frame_buffer[j] = frame_buffer[j] - 0.95f * frame_buffer[j-1];
            }

            /* Step 3: Apply window */
            for (j = 0; j < PIPELINE_FRAME_SIZE; j++) {
                windowed_frame[j] = frame_buffer[j] * hann_window[j];
            }

            /* Step 4: Forward STFT */
            status = vv_dsp_stft_process(stft_handle, windowed_frame, spectrum);
            if (status != VV_DSP_OK) continue;

            /* Step 5: Spectral processing */
            spectral_processing_placeholder(spectrum, PIPELINE_FRAME_SIZE);

            /* Step 6: Inverse STFT with overlap-add */
            status = vv_dsp_stft_reconstruct(stft_handle, spectrum,
                                             processed_signal + frame_idx * PIPELINE_HOP_SIZE,
                                             NULL);
        }
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_samples = (double)(PIPELINE_SIGNAL_LEN * PIPELINE_NUM_ITERATIONS);
    double samples_per_second = total_samples / elapsed;
    double audio_duration = (double)PIPELINE_SIGNAL_LEN / 48000.0;  /* 10 seconds per iteration */
    double total_audio_duration = audio_duration * (double)PIPELINE_NUM_ITERATIONS;
    double rtf = elapsed / total_audio_duration;

    vv_bench_add_result(suite, "Complete_Audio_Pipeline", elapsed, samples_per_second, rtf, PIPELINE_NUM_ITERATIONS);

    /* Cleanup */
    vv_dsp_stft_destroy(stft_handle);
}

static void benchmark_realtime_processing_simulation(vv_bench_suite* suite) {
    /*
     * Simulate real-time processing with small frame buffers
     * Processing 64 samples at a time (1.33ms at 48kHz)
     */

    const size_t realtime_frame_size = 64;   /* Small frame for real-time */
    const size_t num_realtime_frames = PIPELINE_SIGNAL_LEN / realtime_frame_size;

    /* Pre-emphasis coefficient */
    const vv_dsp_real preemph_coeff = 0.97f;
    vv_dsp_real preemph_state = 0.0f;

    /* Simple lowpass filter for post-processing */
    const vv_dsp_real lpf_coeff = 0.1f;
    vv_dsp_real lpf_state = 0.0f;

    /* Generate test audio */
    generate_test_audio(input_signal, PIPELINE_SIGNAL_LEN);

    /* Benchmark real-time processing loop */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    for (iter = 0; iter < PIPELINE_NUM_ITERATIONS; iter++) {
        preemph_state = 0.0f;
        lpf_state = 0.0f;

        size_t frame_idx;
        for (frame_idx = 0; frame_idx < num_realtime_frames; frame_idx++) {
            size_t start_idx = frame_idx * realtime_frame_size;

            /* Process frame sample by sample */
            size_t j;
            for (j = 0; j < realtime_frame_size; j++) {
                if (start_idx + j >= PIPELINE_SIGNAL_LEN) break;

                vv_dsp_real input_sample = input_signal[start_idx + j];

                /* Pre-emphasis */
                vv_dsp_real preemph_output = input_sample - preemph_coeff * preemph_state;
                preemph_state = input_sample;

                /* Simple processing (gain + compression simulation) */
                vv_dsp_real processed = preemph_output * 1.2f;
                if (processed > 1.0f) processed = 1.0f;
                if (processed < -1.0f) processed = -1.0f;

                /* Low-pass filter */
                lpf_state = lpf_state + lpf_coeff * (processed - lpf_state);

                processed_signal[start_idx + j] = lpf_state;
            }
        }
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_samples = (double)(PIPELINE_SIGNAL_LEN * PIPELINE_NUM_ITERATIONS);
    double samples_per_second = total_samples / elapsed;
    double audio_duration = (double)PIPELINE_SIGNAL_LEN / 48000.0;
    double total_audio_duration = audio_duration * (double)PIPELINE_NUM_ITERATIONS;
    double rtf = elapsed / total_audio_duration;

    vv_bench_add_result(suite, "Realtime_Processing_Simulation", elapsed, samples_per_second, rtf, PIPELINE_NUM_ITERATIONS);
}

static void benchmark_memory_intensive_pipeline(vv_bench_suite* suite) {
    /*
     * Test memory-intensive operations: multiple large buffers and operations
     */

    /* Allocate multiple processing buffers */
    vv_dsp_real* temp_buffer1 = (vv_dsp_real*)malloc(PIPELINE_SIGNAL_LEN * sizeof(vv_dsp_real));
    vv_dsp_real* temp_buffer2 = (vv_dsp_real*)malloc(PIPELINE_SIGNAL_LEN * sizeof(vv_dsp_real));
    vv_dsp_real* temp_buffer3 = (vv_dsp_real*)malloc(PIPELINE_SIGNAL_LEN * sizeof(vv_dsp_real));

    if (!temp_buffer1 || !temp_buffer2 || !temp_buffer3) {
        fprintf(stderr, "Failed to allocate memory for memory-intensive pipeline benchmark\n");
        free(temp_buffer1);
        free(temp_buffer2);
        free(temp_buffer3);
        return;
    }

    /* Generate test audio */
    generate_test_audio(input_signal, PIPELINE_SIGNAL_LEN);

    /* Benchmark memory-intensive processing */
    vv_bench_time start = vv_bench_get_time();

    size_t iter;
    for (iter = 0; iter < PIPELINE_NUM_ITERATIONS; iter++) {
        /* Stage 1: Copy and basic processing */
        size_t i;
        for (i = 0; i < PIPELINE_SIGNAL_LEN; i++) {
            temp_buffer1[i] = input_signal[i] * 0.8f;  /* Scale */
        }

        /* Stage 2: More complex processing with memory access patterns */
        for (i = 1; i < PIPELINE_SIGNAL_LEN; i++) {
            temp_buffer2[i] = temp_buffer1[i] - 0.95f * temp_buffer1[i-1];  /* High-pass */
        }
        temp_buffer2[0] = temp_buffer1[0];

        /* Stage 3: Moving average (memory bandwidth test) */
        const size_t avg_window = 5;
        for (i = avg_window; i < PIPELINE_SIGNAL_LEN; i++) {
            vv_dsp_real sum = 0.0f;
            size_t j;
            for (j = 0; j < avg_window; j++) {
                sum += temp_buffer2[i - j];
            }
            temp_buffer3[i] = sum / (vv_dsp_real)avg_window;
        }

        /* Stage 4: Copy result back */
        memcpy(processed_signal, temp_buffer3, PIPELINE_SIGNAL_LEN * sizeof(vv_dsp_real));
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    /* Calculate metrics */
    double total_samples = (double)(PIPELINE_SIGNAL_LEN * PIPELINE_NUM_ITERATIONS);
    double samples_per_second = total_samples / elapsed;
    double audio_duration = (double)PIPELINE_SIGNAL_LEN / 48000.0;
    double total_audio_duration = audio_duration * (double)PIPELINE_NUM_ITERATIONS;
    double rtf = elapsed / total_audio_duration;

    vv_bench_add_result(suite, "Memory_Intensive_Pipeline", elapsed, samples_per_second, rtf, PIPELINE_NUM_ITERATIONS);

    /* Cleanup */
    free(temp_buffer1);
    free(temp_buffer2);
    free(temp_buffer3);
}

void run_pipeline_benchmarks(vv_bench_suite* suite) {
    if (!suite) return;

    /* Run end-to-end pipeline benchmarks */
    benchmark_complete_audio_pipeline(suite);
    benchmark_realtime_processing_simulation(suite);
    benchmark_memory_intensive_pipeline(suite);
}
