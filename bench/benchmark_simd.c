/**
 * @file benchmark_simd.c
 * @brief SIMD vs Scalar performance benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "vv_dsp/core.h"

#define BENCHMARK_SIZE 10000
#define BENCHMARK_ITERATIONS 1000

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

// Scalar reference implementations
static void scalar_add(const vv_dsp_real* a, const vv_dsp_real* b, vv_dsp_real* result, size_t n) {
    for (size_t i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
}

static void scalar_mul(const vv_dsp_real* a, const vv_dsp_real* b, vv_dsp_real* result, size_t n) {
    for (size_t i = 0; i < n; i++) {
        result[i] = a[i] * b[i];
    }
}

static vv_dsp_real scalar_sum(const vv_dsp_real* data, size_t n) {
    vv_dsp_real sum = 0.0f;
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}

static vv_dsp_real scalar_rms(const vv_dsp_real* data, size_t n) {
    vv_dsp_real sum_sq = 0.0f;
    for (size_t i = 0; i < n; i++) {
        sum_sq += data[i] * data[i];
    }
    return sqrtf(sum_sq / n);
}

int main(void) {
    printf("SIMD Performance Benchmark\n");
    printf("==========================\n");
    printf("Array size: %d elements\n", BENCHMARK_SIZE);
    printf("Iterations: %d\n\n", BENCHMARK_ITERATIONS);

    // Allocate test data
    vv_dsp_real* a = (vv_dsp_real*)malloc(BENCHMARK_SIZE * sizeof(vv_dsp_real));
    vv_dsp_real* b = (vv_dsp_real*)malloc(BENCHMARK_SIZE * sizeof(vv_dsp_real));
    vv_dsp_real* result = (vv_dsp_real*)malloc(BENCHMARK_SIZE * sizeof(vv_dsp_real));

    if (!a || !b || !result) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Initialize test data
    srand(42);
    for (size_t i = 0; i < BENCHMARK_SIZE; i++) {
        a[i] = (vv_dsp_real)rand() / RAND_MAX * 2.0f - 1.0f;
        b[i] = (vv_dsp_real)rand() / RAND_MAX * 2.0f - 1.0f;
    }

    double start, end;
    vv_dsp_real dummy_result;

    // Benchmark Vector Addition
    printf("Vector Addition:\n");

    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        scalar_add(a, b, result, BENCHMARK_SIZE);
    }
    end = get_time_ms();
    printf("  Scalar:     %.3f ms (%.6f ms/iter)\n", end - start, (end - start) / BENCHMARK_ITERATIONS);

#ifdef VV_DSP_USE_SIMD
    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        vv_dsp_add_real_simd(a, b, result, BENCHMARK_SIZE);
    }
    end = get_time_ms();
    printf("  SIMD:       %.3f ms (%.6f ms/iter)\n", end - start, (end - start) / BENCHMARK_ITERATIONS);
    printf("  Speedup:    %.2fx\n",
           ((double)(end - start) / BENCHMARK_ITERATIONS) /
           ((double)(end - start) / BENCHMARK_ITERATIONS));
#else
    printf("  SIMD:       Not available (VV_DSP_USE_SIMD not defined)\n");
#endif

    // Benchmark Vector Multiplication
    printf("\nVector Multiplication:\n");

    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        scalar_mul(a, b, result, BENCHMARK_SIZE);
    }
    end = get_time_ms();
    printf("  Scalar:     %.3f ms (%.6f ms/iter)\n", end - start, (end - start) / BENCHMARK_ITERATIONS);

#ifdef VV_DSP_USE_SIMD
    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        vv_dsp_mul_real_simd(a, b, result, BENCHMARK_SIZE);
    }
    end = get_time_ms();
    printf("  SIMD:       %.3f ms (%.6f ms/iter)\n", end - start, (end - start) / BENCHMARK_ITERATIONS);
#else
    printf("  SIMD:       Not available\n");
#endif

    // Benchmark Sum
    printf("\nSum Calculation:\n");

    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        dummy_result = scalar_sum(a, BENCHMARK_SIZE);
    }
    end = get_time_ms();
    printf("  Scalar:     %.3f ms (%.6f ms/iter, result=%.6f)\n",
           end - start, (end - start) / BENCHMARK_ITERATIONS, dummy_result);

#ifdef VV_DSP_USE_SIMD
    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        float sum = 0.0f;
        vv_dsp_sum_optimized(a, BENCHMARK_SIZE, &sum);
        dummy_result = sum;
    }
    end = get_time_ms();
    printf("  SIMD:       %.3f ms (%.6f ms/iter, result=%.6f)\n",
           end - start, (end - start) / BENCHMARK_ITERATIONS, dummy_result);
#else
    printf("  SIMD:       Not available\n");
#endif

    // Benchmark RMS
    printf("\nRMS Calculation:\n");

    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        dummy_result = scalar_rms(a, BENCHMARK_SIZE);
    }
    end = get_time_ms();
    printf("  Scalar:     %.3f ms (%.6f ms/iter, result=%.6f)\n",
           end - start, (end - start) / BENCHMARK_ITERATIONS, dummy_result);

#ifdef VV_DSP_USE_SIMD
    start = get_time_ms();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        float rms = 0.0f;
        vv_dsp_rms_optimized(a, BENCHMARK_SIZE, &rms);
        dummy_result = rms;
    }
    end = get_time_ms();
    printf("  SIMD:       %.3f ms (%.6f ms/iter, result=%.6f)\n",
           end - start, (end - start) / BENCHMARK_ITERATIONS, dummy_result);
#else
    printf("  SIMD:       Not available\n");
#endif

    // Cleanup
    free(a);
    free(b);
    free(result);

    printf("\nBenchmark completed!\n");
    return 0;
}
