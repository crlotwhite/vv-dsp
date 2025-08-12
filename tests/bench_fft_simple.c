/**
 * @file bench_fft_simple.c
 * @brief Simple C-only micro-benchmark for FFT operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "vv_dsp/vv_dsp.h"

#define BENCHMARK_SIZE 1024
#define BENCHMARK_ITERATIONS 1000

static double get_time_ms(void) {
#ifdef _WIN32
    /* Windows-specific high-resolution timer */
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;

    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&counter);
    return (double)(counter.QuadPart * 1000) / frequency.QuadPart;
#else
    /* POSIX systems */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
#endif
}

static void benchmark_fft(void) {
    vv_dsp_cpx* input = malloc(BENCHMARK_SIZE * sizeof(vv_dsp_cpx));
    vv_dsp_cpx* output = malloc(BENCHMARK_SIZE * sizeof(vv_dsp_cpx));

    if (!input || !output) {
        printf("Memory allocation failed\n");
        return;
    }

    /* Initialize with some test data */
    for (size_t i = 0; i < BENCHMARK_SIZE; i++) {
        input[i].re = (vv_dsp_real)i / BENCHMARK_SIZE;
        input[i].im = 0.0f;
    }

    /* Create FFT plan */
    vv_dsp_fft_plan* plan = NULL;
    vv_dsp_status status = vv_dsp_fft_make_plan(BENCHMARK_SIZE, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);
    if (status != VV_DSP_OK) {
        printf("Failed to create FFT plan: %d\n", status);
        free(input);
        free(output);
        return;
    }

    printf("Running FFT benchmark (size=%d, iterations=%d)...\n",
           BENCHMARK_SIZE, BENCHMARK_ITERATIONS);

    double start_time = get_time_ms();

    for (int iter = 0; iter < BENCHMARK_ITERATIONS; iter++) {
        status = vv_dsp_fft_execute(plan, input, output);
        if (status != VV_DSP_OK) {
            printf("FFT failed with status: %d\n", status);
            break;
        }
    }

    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    double time_per_fft = total_time / BENCHMARK_ITERATIONS;

    printf("Total time: %.2f ms\n", total_time);
    printf("Time per FFT: %.3f ms\n", time_per_fft);
    printf("FFTs per second: %.0f\n", 1000.0 / time_per_fft);

    vv_dsp_fft_destroy(plan);
    free(input);
    free(output);
}

int main(void) {
    printf("VV-DSP C-only Benchmark\n");
    printf("========================\n");

    benchmark_fft();

    return 0;
}
