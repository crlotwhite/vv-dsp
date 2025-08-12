/**
 * @file bench_denormals.c
 * @brief Benchmark to validate FTZ/DAZ performance impact on denormal processing
 * @ingroup benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "vv_dsp/vv_dsp.h"
#include "vv_dsp/core/fp_env.h"
#include "bench_framework.h"
#include "bench_timer.h"

/* Test parameters */
#define DENORMAL_BENCH_SIZE     (1024 * 1024)  /* 1M samples */
#define DENORMAL_BENCH_WARMUP   10
#define DENORMAL_BENCH_ITER     100

/* Global test data */
static float *denormal_input = NULL;
static float *denormal_output = NULL;
static float *denormal_state = NULL;

/**
 * @brief Initialize test data with denormal numbers
 */
static int init_denormal_data(void) {
    size_t i;

    denormal_input = (float*)malloc(DENORMAL_BENCH_SIZE * sizeof(float));
    denormal_output = (float*)malloc(DENORMAL_BENCH_SIZE * sizeof(float));
    denormal_state = (float*)malloc(4 * sizeof(float));  /* IIR filter state */

    if (!denormal_input || !denormal_output || !denormal_state) {
        fprintf(stderr, "Failed to allocate memory for denormal benchmark\n");
        return -1;
    }

    /* Fill input with denormal numbers and small values that will decay to denormals */
    for (i = 0; i < DENORMAL_BENCH_SIZE; i++) {
        if (i % 4 == 0) {
            /* Pure denormal numbers */
            denormal_input[i] = FLT_MIN / powf(2.0f, (float)(i % 20 + 1));
        } else if (i % 4 == 1) {
            /* Small normal numbers that will become denormal after processing */
            denormal_input[i] = FLT_MIN * powf(2.0f, -(float)(i % 10));
        } else if (i % 4 == 2) {
            /* Zero (for contrast) */
            denormal_input[i] = 0.0f;
        } else {
            /* Very small but normal numbers */
            denormal_input[i] = FLT_MIN * 2.0f * sinf((float)i * 0.001f);
        }
    }

    /* Initialize filter state with denormals */
    denormal_state[0] = FLT_MIN / 8.0f;
    denormal_state[1] = FLT_MIN / 16.0f;
    denormal_state[2] = FLT_MIN / 32.0f;
    denormal_state[3] = FLT_MIN / 64.0f;

    return 0;
}

/**
 * @brief Clean up test data
 */
static void cleanup_denormal_data(void) {
    free(denormal_input);
    free(denormal_output);
    free(denormal_state);
    denormal_input = NULL;
    denormal_output = NULL;
    denormal_state = NULL;
}

/**
 * @brief IIR filter processing that generates many denormals
 *
 * This implements a simple recursive filter that tends to produce
 * denormal intermediate values, which is known to be slow without FTZ.
 */
static void denormal_heavy_iir_filter(void) {
    size_t i;
    float x, y;
    volatile float s0 = denormal_state[0];
    volatile float s1 = denormal_state[1];
    volatile float s2 = denormal_state[2];
    volatile float s3 = denormal_state[3];

    /* Coefficients designed to create feedback with denormals */
    const float a1 = 0.99f;    /* Strong feedback */
    const float a2 = -0.98f;   /* Creates oscillation that decays to denormals */
    const float b0 = 0.001f;   /* Small gain to promote denormals */
    const float b1 = 0.0005f;

    for (i = 0; i < DENORMAL_BENCH_SIZE; i++) {
        x = denormal_input[i];

        /* Direct form I IIR filter: y[n] = b0*x[n] + b1*x[n-1] + a1*y[n-1] + a2*y[n-2] */
        y = b0 * x + b1 * s0 + a1 * s1 + a2 * s2;

        /* Update state */
        s0 = x;      /* x[n-1] */
        s2 = s1;     /* y[n-2] */
        s1 = y;      /* y[n-1] */

        /* Additional denormal-generating operations */
        s3 = s3 * 0.999f + y * 0.0001f;  /* Slowly decaying state */

        denormal_output[i] = y + s3;
    }

    /* Save state for next iteration */
    denormal_state[0] = s0;
    denormal_state[1] = s1;
    denormal_state[2] = s2;
    denormal_state[3] = s3;
}

/**
 * @brief Multiplication-heavy denormal processing
 */
static void denormal_heavy_multiply(void) {
    size_t i;
    volatile float acc = FLT_MIN / 1024.0f;  /* Start with denormal */

    for (i = 0; i < DENORMAL_BENCH_SIZE; i++) {
        /* Chain of operations that maintain denormals */
        acc = acc * 0.9999f + denormal_input[i] * 0.0001f;
        acc = acc * 1.0001f - denormal_input[i] * 0.00005f;
        denormal_output[i] = acc;

        /* Prevent accumulation from going to zero or infinity */
        if (i % 1000 == 999) {
            if (fabsf(acc) > 1.0f) acc *= 0.001f;
            if (acc == 0.0f) acc = FLT_MIN / 2048.0f;
        }
    }
}

/**
 * @brief Run denormal benchmark with current FTZ state
 */
static double run_denormal_benchmark_internal(void (*bench_func)(void), const char* name) {
    vv_bench_time start, end;
    size_t iter;
    double total_time = 0.0;

    (void)name;  /* Suppress unused parameter warning */

    /* Warmup */
    for (iter = 0; iter < DENORMAL_BENCH_WARMUP; iter++) {
        bench_func();
    }

    /* Timed iterations */
    for (iter = 0; iter < DENORMAL_BENCH_ITER; iter++) {
        start = vv_bench_get_time();
        bench_func();
        end = vv_bench_get_time();
        total_time += vv_bench_elapsed_seconds(start, end);
    }

    /* Calculate samples per second */
    double avg_time = total_time / (double)DENORMAL_BENCH_ITER;
    return (double)DENORMAL_BENCH_SIZE / avg_time;
}

/**
 * @brief Run IIR filter benchmark comparing FTZ on/off performance
 */
static void bench_denormal_iir_filter(vv_bench_suite* suite) {
    double samples_per_sec_normal, samples_per_sec_ftz;
    double speedup_factor;
    bool initial_ftz_state;

    printf("Running denormal IIR filter benchmark...\n");

    /* Save initial FTZ state */
    initial_ftz_state = vv_dsp_get_flush_denormals_mode();

    /* Test with FTZ disabled (normal IEEE 754 behavior) */
    vv_dsp_set_flush_denormals(false);
    samples_per_sec_normal = run_denormal_benchmark_internal(denormal_heavy_iir_filter, "denormal_iir_normal");

    /* Reset state between tests */
    denormal_state[0] = FLT_MIN / 8.0f;
    denormal_state[1] = FLT_MIN / 16.0f;
    denormal_state[2] = FLT_MIN / 32.0f;
    denormal_state[3] = FLT_MIN / 64.0f;

    /* Test with FTZ enabled */
    vv_dsp_set_flush_denormals(true);
    samples_per_sec_ftz = run_denormal_benchmark_internal(denormal_heavy_iir_filter, "denormal_iir_ftz");

    /* Calculate speedup */
    speedup_factor = samples_per_sec_ftz / samples_per_sec_normal;

    /* Add results to benchmark suite */
    vv_bench_add_result(suite, "denormal_iir_normal_mode",
                       (double)DENORMAL_BENCH_SIZE / samples_per_sec_normal,
                       samples_per_sec_normal, 0.0, DENORMAL_BENCH_ITER);

    vv_bench_add_result(suite, "denormal_iir_ftz_mode",
                       (double)DENORMAL_BENCH_SIZE / samples_per_sec_ftz,
                       samples_per_sec_ftz, 0.0, DENORMAL_BENCH_ITER);

    /* Print summary */
    printf("IIR Filter Denormal Benchmark Results:\n");
    printf("  Normal mode: %.2f M samples/sec\n", samples_per_sec_normal / 1e6);
    printf("  FTZ mode:    %.2f M samples/sec\n", samples_per_sec_ftz / 1e6);
    printf("  Speedup:     %.2fx\n", speedup_factor);

    if (speedup_factor < 2.0) {
        printf("  WARNING: Expected significant speedup (>2x) not observed!\n");
    } else {
        printf("  SUCCESS: FTZ provides significant performance improvement\n");
    }

    /* Restore initial FTZ state */
    vv_dsp_set_flush_denormals(initial_ftz_state);
}

/**
 * @brief Run multiplication benchmark comparing FTZ on/off performance
 */
static void bench_denormal_multiply(vv_bench_suite* suite) {
    double samples_per_sec_normal, samples_per_sec_ftz;
    double speedup_factor;
    bool initial_ftz_state;

    printf("Running denormal multiplication benchmark...\n");

    /* Save initial FTZ state */
    initial_ftz_state = vv_dsp_get_flush_denormals_mode();

    /* Test with FTZ disabled */
    vv_dsp_set_flush_denormals(false);
    samples_per_sec_normal = run_denormal_benchmark_internal(denormal_heavy_multiply, "denormal_mult_normal");

    /* Test with FTZ enabled */
    vv_dsp_set_flush_denormals(true);
    samples_per_sec_ftz = run_denormal_benchmark_internal(denormal_heavy_multiply, "denormal_mult_ftz");

    /* Calculate speedup */
    speedup_factor = samples_per_sec_ftz / samples_per_sec_normal;

    /* Add results to benchmark suite */
    vv_bench_add_result(suite, "denormal_mult_normal_mode",
                       (double)DENORMAL_BENCH_SIZE / samples_per_sec_normal,
                       samples_per_sec_normal, 0.0, DENORMAL_BENCH_ITER);

    vv_bench_add_result(suite, "denormal_mult_ftz_mode",
                       (double)DENORMAL_BENCH_SIZE / samples_per_sec_ftz,
                       samples_per_sec_ftz, 0.0, DENORMAL_BENCH_ITER);

    /* Print summary */
    printf("Multiplication Denormal Benchmark Results:\n");
    printf("  Normal mode: %.2f M samples/sec\n", samples_per_sec_normal / 1e6);
    printf("  FTZ mode:    %.2f M samples/sec\n", samples_per_sec_ftz / 1e6);
    printf("  Speedup:     %.2fx\n", speedup_factor);

    if (speedup_factor < 1.5) {
        printf("  WARNING: Expected moderate speedup (>1.5x) not observed!\n");
    } else {
        printf("  SUCCESS: FTZ provides performance improvement\n");
    }

    /* Restore initial FTZ state */
    vv_dsp_set_flush_denormals(initial_ftz_state);
}

/**
 * @brief Main entry point for denormal benchmarks
 */
void run_denormal_benchmarks(vv_bench_suite* suite) {
    printf("\n=== Denormal Processing Performance Benchmarks ===\n");

    /* Initialize test data */
    if (init_denormal_data() != 0) {
        fprintf(stderr, "Failed to initialize denormal benchmark data\n");
        return;
    }

    printf("Testing with %zu samples, %d iterations\n",
           (size_t)DENORMAL_BENCH_SIZE, DENORMAL_BENCH_ITER);
    printf("Platform FTZ support: %s\n",
           vv_dsp_get_flush_denormals_mode() ? "Available" : "Limited/None");

    /* Run benchmarks */
    bench_denormal_iir_filter(suite);
    printf("\n");
    bench_denormal_multiply(suite);

    /* Cleanup */
    cleanup_denormal_data();

    printf("\n=== Denormal Benchmarks Complete ===\n");
}
