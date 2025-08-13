/**
 * @file bench_vectorized_math.c
 * @brief Benchmark vectorized math operations against scalar implementations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "vv_dsp/core/vv_dsp_vectorized_math.h"
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/vv_dsp_types.h"

// Simple timing utility
static double get_time_seconds(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

// Reference scalar window apply implementation
static void scalar_window_apply(const vv_dsp_real* in, const vv_dsp_real* window,
                               vv_dsp_real* out, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        out[i] = in[i] * window[i];
    }
}

// Reference scalar complex multiply implementation
static void scalar_complex_multiply(const vv_dsp_cpx* a, const vv_dsp_cpx* b,
                                   vv_dsp_cpx* result, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        const vv_dsp_real a_re = a[i].re;
        const vv_dsp_real a_im = a[i].im;
        const vv_dsp_real b_re = b[i].re;
        const vv_dsp_real b_im = b[i].im;

        result[i].re = a_re * b_re - a_im * b_im;
        result[i].im = a_re * b_im + a_im * b_re;
    }
}

// Reference scalar trigonometric apply
static void scalar_trig_apply(const vv_dsp_real* in, vv_dsp_real* out,
                             size_t n, int func_type) {
    for (size_t i = 0; i < n; ++i) {
        switch (func_type) {
            case 0: out[i] = VV_DSP_SIN(in[i]); break;
            case 1: out[i] = VV_DSP_COS(in[i]); break;
            case 2: out[i] = VV_DSP_TAN(in[i]); break;
        }
    }
}

// Verify correctness of vectorized implementation
static int verify_window_apply(size_t n) {
    vv_dsp_real* in = malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* window = malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* out_scalar = malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* out_vector = malloc(n * sizeof(vv_dsp_real));

    if (!in || !window || !out_scalar || !out_vector) {
        free(in); free(window); free(out_scalar); free(out_vector);
        return 0;
    }

    // Initialize with test data
    for (size_t i = 0; i < n; ++i) {
        in[i] = (vv_dsp_real)sin(2.0 * M_PI * i / n);
        window[i] = (vv_dsp_real)(0.5 - 0.5 * cos(2.0 * M_PI * i / n)); // Hann window
    }

    // Compute both implementations
    scalar_window_apply(in, window, out_scalar, n);
    vv_dsp_vectorized_window_apply(in, window, out_vector, n);

    // Verify results match within tolerance
    const vv_dsp_real tolerance = 1e-6f;
    int passed = 1;
    for (size_t i = 0; i < n; ++i) {
        vv_dsp_real diff = (vv_dsp_real)fabs(out_scalar[i] - out_vector[i]);
        if (diff > tolerance) {
            printf("Window apply verification failed at index %zu: scalar=%f, vector=%f, diff=%f\n",
                   i, out_scalar[i], out_vector[i], diff);
            passed = 0;
            break;
        }
    }

    free(in); free(window); free(out_scalar); free(out_vector);
    return passed;
}

// Benchmark window apply operations
static void benchmark_window_apply(size_t n, int iterations) {
    vv_dsp_real* in = malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* window = malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* out = malloc(n * sizeof(vv_dsp_real));

    if (!in || !window || !out) {
        free(in); free(window); free(out);
        return;
    }

    // Initialize data
    for (size_t i = 0; i < n; ++i) {
        in[i] = (vv_dsp_real)sin(2.0 * M_PI * i / n);
        window[i] = (vv_dsp_real)(0.5 - 0.5 * cos(2.0 * M_PI * i / n));
    }

    printf("Window Apply Benchmark (size=%zu, iterations=%d):\n", n, iterations);

    // Benchmark scalar implementation
    double scalar_start = get_time_seconds();
    for (int iter = 0; iter < iterations; ++iter) {
        scalar_window_apply(in, window, out, n);
    }
    double scalar_time = get_time_seconds() - scalar_start;

    // Benchmark vectorized implementation
    double vector_start = get_time_seconds();
    for (int iter = 0; iter < iterations; ++iter) {
        vv_dsp_vectorized_window_apply(in, window, out, n);
    }
    double vector_time = get_time_seconds() - vector_start;

    printf("  Scalar:     %.6f seconds (%.2f ns/sample)\n",
           scalar_time, scalar_time * 1e9 / (iterations * n));
    printf("  Vectorized: %.6f seconds (%.2f ns/sample)\n",
           vector_time, vector_time * 1e9 / (iterations * n));
    printf("  Speedup:    %.2fx\n", scalar_time / vector_time);
    printf("\n");

    free(in); free(window); free(out);
}

// Benchmark complex multiply operations
static void benchmark_complex_multiply(size_t n, int iterations) {
    vv_dsp_cpx* a = malloc(n * sizeof(vv_dsp_cpx));
    vv_dsp_cpx* b = malloc(n * sizeof(vv_dsp_cpx));
    vv_dsp_cpx* result = malloc(n * sizeof(vv_dsp_cpx));

    if (!a || !b || !result) {
        free(a); free(b); free(result);
        return;
    }

    // Initialize data
    for (size_t i = 0; i < n; ++i) {
        a[i].re = (vv_dsp_real)cos(2.0 * M_PI * i / n);
        a[i].im = (vv_dsp_real)sin(2.0 * M_PI * i / n);
        b[i].re = (vv_dsp_real)cos(2.0 * M_PI * (i + n/4) / n);
        b[i].im = (vv_dsp_real)sin(2.0 * M_PI * (i + n/4) / n);
    }

    printf("Complex Multiply Benchmark (size=%zu, iterations=%d):\n", n, iterations);

    // Benchmark scalar implementation
    double scalar_start = get_time_seconds();
    for (int iter = 0; iter < iterations; ++iter) {
        scalar_complex_multiply(a, b, result, n);
    }
    double scalar_time = get_time_seconds() - scalar_start;

    // Benchmark vectorized implementation
    double vector_start = get_time_seconds();
    for (int iter = 0; iter < iterations; ++iter) {
        vv_dsp_vectorized_complex_multiply(a, b, result, n);
    }
    double vector_time = get_time_seconds() - vector_start;

    printf("  Scalar:     %.6f seconds (%.2f ns/sample)\n",
           scalar_time, scalar_time * 1e9 / (iterations * n));
    printf("  Vectorized: %.6f seconds (%.2f ns/sample)\n",
           vector_time, vector_time * 1e9 / (iterations * n));
    printf("  Speedup:    %.2fx\n", scalar_time / vector_time);
    printf("\n");

    free(a); free(b); free(result);
}

// Benchmark trigonometric operations
static void benchmark_trig_apply(size_t n, int iterations, int func_type) {
    vv_dsp_real* in = malloc(n * sizeof(vv_dsp_real));
    vv_dsp_real* out = malloc(n * sizeof(vv_dsp_real));

    if (!in || !out) {
        free(in); free(out);
        return;
    }

    // Initialize data
    for (size_t i = 0; i < n; ++i) {
        in[i] = (vv_dsp_real)(2.0 * M_PI * i / n - M_PI); // Range [-π, π]
    }

    const char* func_names[] = {"sin", "cos", "tan"};
    printf("%s Benchmark (size=%zu, iterations=%d):\n", func_names[func_type], n, iterations);

    // Benchmark scalar implementation
    double scalar_start = get_time_seconds();
    for (int iter = 0; iter < iterations; ++iter) {
        scalar_trig_apply(in, out, n, func_type);
    }
    double scalar_time = get_time_seconds() - scalar_start;

    // Benchmark vectorized implementation
    double vector_start = get_time_seconds();
    for (int iter = 0; iter < iterations; ++iter) {
        vv_dsp_vectorized_trig_apply(in, out, n, func_type);
    }
    double vector_time = get_time_seconds() - vector_start;

    printf("  Scalar:     %.6f seconds (%.2f ns/sample)\n",
           scalar_time, scalar_time * 1e9 / (iterations * n));
    printf("  Vectorized: %.6f seconds (%.2f ns/sample)\n",
           vector_time, vector_time * 1e9 / (iterations * n));
    printf("  Speedup:    %.2fx\n", scalar_time / vector_time);
    printf("\n");

    free(in); free(out);
}

int main(void) {
    printf("Vectorized Math Benchmark\n");
    printf("=========================\n");
    printf("Eigen vectorization available: %s\n\n",
           vv_dsp_vectorized_math_available() ? "Yes" : "No");

    // Test sizes representing typical DSP use cases
    const size_t test_sizes[] = {256, 512, 1024, 2048, 4096, 8192};
    const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int iterations = 10000;

    // Verify correctness first
    printf("Correctness Verification:\n");
    for (int i = 0; i < num_sizes; ++i) {
        if (verify_window_apply(test_sizes[i])) {
            printf("  Window apply size %zu: PASSED\n", test_sizes[i]);
        } else {
            printf("  Window apply size %zu: FAILED\n", test_sizes[i]);
            return 1;
        }
    }
    printf("\n");

    // Run benchmarks for different sizes
    for (int i = 0; i < num_sizes; ++i) {
        size_t n = test_sizes[i];

        benchmark_window_apply(n, iterations);
        benchmark_complex_multiply(n, iterations / 10); // Fewer iterations for complex ops
        benchmark_trig_apply(n, iterations, 0); // sin
        benchmark_trig_apply(n, iterations, 1); // cos

        printf("----------------------------------------\n");
    }

    return 0;
}
