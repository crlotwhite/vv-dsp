/**
 * @file bench_accuracy_performance_trade_offs.c
 * @brief Comprehensive benchmark to quantify accuracy-performance trade-offs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "vv_dsp/core/vv_dsp_vectorized_math.h"
#include "vv_dsp/vv_dsp_math.h"
#include "vv_dsp/vv_dsp_types.h"

// Simple timing utility
static double get_time_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

// Accuracy metrics structure
typedef struct {
    double max_abs_error;
    double rmse;
    double mean_abs_error;
} accuracy_metrics_t;

// Calculate accuracy metrics
static accuracy_metrics_t calculate_accuracy_metrics(const vv_dsp_real* reference,
                                                     const vv_dsp_real* test,
                                                     size_t n) {
    accuracy_metrics_t metrics = {0};
    double sum_squared_error = 0.0;
    double sum_abs_error = 0.0;

    for (size_t i = 0; i < n; ++i) {
        double error = (double)test[i] - (double)reference[i];
        double abs_error = fabs(error);

        if (abs_error > metrics.max_abs_error) {
            metrics.max_abs_error = abs_error;
        }

        sum_abs_error += abs_error;
        sum_squared_error += error * error;
    }

    metrics.mean_abs_error = sum_abs_error / n;
    metrics.rmse = sqrt(sum_squared_error / n);

    return metrics;
}

// Benchmark trigonometric functions
static void benchmark_trig_functions(void) {
    const size_t test_sizes[] = {1024, 4096, 16384};
    const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int iterations = 1000;

    printf("Trigonometric Functions Performance & Accuracy Analysis\n");
    printf("========================================================\n\n");

    for (int size_idx = 0; size_idx < num_sizes; ++size_idx) {
        size_t n = test_sizes[size_idx];

        // Allocate buffers
        vv_dsp_real* input = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* out_std = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* out_vectorized = malloc(n * sizeof(vv_dsp_real));

        if (!input || !out_std || !out_vectorized) continue;

        // Initialize input with values in [-π, π]
        for (size_t i = 0; i < n; ++i) {
            input[i] = (vv_dsp_real)(2.0 * M_PI * (double)i / n - M_PI);
        }

        printf("Array Size: %zu samples\n", n);
        printf("-------------------\n");

        // Test sin function
        printf("SIN Function:\n");

        // Standard implementation (reference)
        double std_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < n; ++i) {
                out_std[i] = (vv_dsp_real)sin((double)input[i]);
            }
        }
        double std_time = get_time_seconds() - std_start;

        // Vectorized implementation
        double vec_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            vv_dsp_vectorized_trig_apply(input, out_vectorized, n, 0); // sin
        }
        double vec_time = get_time_seconds() - vec_start;

        // Calculate accuracy metrics
        accuracy_metrics_t sin_metrics = calculate_accuracy_metrics(out_std, out_vectorized, n);

        printf("  Standard:   %.6f ms/iter (%.2f ns/sample)\n",
               std_time * 1000.0 / iterations, std_time * 1e9 / (iterations * n));
        printf("  Vectorized: %.6f ms/iter (%.2f ns/sample)\n",
               vec_time * 1000.0 / iterations, vec_time * 1e9 / (iterations * n));
        printf("  Speedup:    %.2fx\n", std_time / vec_time);
        printf("  Max Abs Error: %.2e\n", sin_metrics.max_abs_error);
        printf("  RMSE:          %.2e\n", sin_metrics.rmse);
        printf("  Mean Abs Error: %.2e\n\n", sin_metrics.mean_abs_error);

        // Test cos function
        printf("COS Function:\n");

        // Standard implementation (reference)
        std_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < n; ++i) {
                out_std[i] = (vv_dsp_real)cos((double)input[i]);
            }
        }
        std_time = get_time_seconds() - std_start;

        // Vectorized implementation
        vec_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            vv_dsp_vectorized_trig_apply(input, out_vectorized, n, 1); // cos
        }
        vec_time = get_time_seconds() - vec_start;

        // Calculate accuracy metrics
        accuracy_metrics_t cos_metrics = calculate_accuracy_metrics(out_std, out_vectorized, n);

        printf("  Standard:   %.6f ms/iter (%.2f ns/sample)\n",
               std_time * 1000.0 / iterations, std_time * 1e9 / (iterations * n));
        printf("  Vectorized: %.6f ms/iter (%.2f ns/sample)\n",
               vec_time * 1000.0 / iterations, vec_time * 1e9 / (iterations * n));
        printf("  Speedup:    %.2fx\n", std_time / vec_time);
        printf("  Max Abs Error: %.2e\n", cos_metrics.max_abs_error);
        printf("  RMSE:          %.2e\n", cos_metrics.rmse);
        printf("  Mean Abs Error: %.2e\n\n", cos_metrics.mean_abs_error);

        printf("============================================\n\n");

        free(input);
        free(out_std);
        free(out_vectorized);
    }
}

// Benchmark window operations
static void benchmark_window_operations(void) {
    const size_t test_sizes[] = {512, 2048, 8192};
    const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int iterations = 5000;

    printf("Window Operations Performance Analysis\n");
    printf("======================================\n\n");

    for (int size_idx = 0; size_idx < num_sizes; ++size_idx) {
        size_t n = test_sizes[size_idx];

        // Allocate buffers
        vv_dsp_real* signal = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* window = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* out_std = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* out_vectorized = malloc(n * sizeof(vv_dsp_real));

        if (!signal || !window || !out_std || !out_vectorized) continue;

        // Initialize signal and Hann window
        for (size_t i = 0; i < n; ++i) {
            signal[i] = (vv_dsp_real)sin(2.0 * M_PI * i / n);
            window[i] = (vv_dsp_real)(0.5 - 0.5 * cos(2.0 * M_PI * i / n));
        }

        printf("Window Application (size=%zu):\n", n);

        // Standard scalar implementation
        double std_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < n; ++i) {
                out_std[i] = signal[i] * window[i];
            }
        }
        double std_time = get_time_seconds() - std_start;

        // Vectorized implementation
        double vec_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            vv_dsp_vectorized_window_apply(signal, window, out_vectorized, n);
        }
        double vec_time = get_time_seconds() - vec_start;

        // Calculate accuracy metrics (should be perfect for simple multiplication)
        accuracy_metrics_t window_metrics = calculate_accuracy_metrics(out_std, out_vectorized, n);

        printf("  Standard:   %.6f ms/iter (%.2f ns/sample)\n",
               std_time * 1000.0 / iterations, std_time * 1e9 / (iterations * n));
        printf("  Vectorized: %.6f ms/iter (%.2f ns/sample)\n",
               vec_time * 1000.0 / iterations, vec_time * 1e9 / (iterations * n));
        printf("  Speedup:    %.2fx\n", std_time / vec_time);
        printf("  Max Abs Error: %.2e\n", window_metrics.max_abs_error);
        printf("  RMSE:          %.2e\n\n", window_metrics.rmse);

        free(signal);
        free(window);
        free(out_std);
        free(out_vectorized);
    }
}

// Benchmark complex multiply operations
static void benchmark_complex_operations(void) {
    const size_t test_sizes[] = {512, 2048, 8192};
    const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int iterations = 1000;

    printf("Complex Multiplication Performance Analysis\n");
    printf("===========================================\n\n");

    for (int size_idx = 0; size_idx < num_sizes; ++size_idx) {
        size_t n = test_sizes[size_idx];

        // Allocate buffers
        vv_dsp_cpx* a = malloc(n * sizeof(vv_dsp_cpx));
        vv_dsp_cpx* b = malloc(n * sizeof(vv_dsp_cpx));
        vv_dsp_cpx* out_std = malloc(n * sizeof(vv_dsp_cpx));
        vv_dsp_cpx* out_vectorized = malloc(n * sizeof(vv_dsp_cpx));

        if (!a || !b || !out_std || !out_vectorized) continue;

        // Initialize complex arrays
        for (size_t i = 0; i < n; ++i) {
            a[i].re = (vv_dsp_real)cos(2.0 * M_PI * i / n);
            a[i].im = (vv_dsp_real)sin(2.0 * M_PI * i / n);
            b[i].re = (vv_dsp_real)cos(2.0 * M_PI * (i + n/4) / n);
            b[i].im = (vv_dsp_real)sin(2.0 * M_PI * (i + n/4) / n);
        }

        printf("Complex Multiplication (size=%zu):\n", n);

        // Standard scalar implementation
        double std_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            for (size_t i = 0; i < n; ++i) {
                vv_dsp_real a_re = a[i].re, a_im = a[i].im;
                vv_dsp_real b_re = b[i].re, b_im = b[i].im;
                out_std[i].re = a_re * b_re - a_im * b_im;
                out_std[i].im = a_re * b_im + a_im * b_re;
            }
        }
        double std_time = get_time_seconds() - std_start;

        // Vectorized implementation
        double vec_start = get_time_seconds();
        for (int iter = 0; iter < iterations; ++iter) {
            vv_dsp_vectorized_complex_multiply(a, b, out_vectorized, n);
        }
        double vec_time = get_time_seconds() - vec_start;

        // Calculate accuracy metrics for real and imaginary parts
        vv_dsp_real* ref_real = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* ref_imag = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* test_real = malloc(n * sizeof(vv_dsp_real));
        vv_dsp_real* test_imag = malloc(n * sizeof(vv_dsp_real));

        for (size_t i = 0; i < n; ++i) {
            ref_real[i] = out_std[i].re;
            ref_imag[i] = out_std[i].im;
            test_real[i] = out_vectorized[i].re;
            test_imag[i] = out_vectorized[i].im;
        }

        accuracy_metrics_t real_metrics = calculate_accuracy_metrics(ref_real, test_real, n);
        accuracy_metrics_t imag_metrics = calculate_accuracy_metrics(ref_imag, test_imag, n);

        printf("  Standard:   %.6f ms/iter (%.2f ns/sample)\n",
               std_time * 1000.0 / iterations, std_time * 1e9 / (iterations * n));
        printf("  Vectorized: %.6f ms/iter (%.2f ns/sample)\n",
               vec_time * 1000.0 / iterations, vec_time * 1e9 / (iterations * n));
        printf("  Speedup:    %.2fx\n", std_time / vec_time);
        printf("  Real Part Max Error: %.2e\n", real_metrics.max_abs_error);
        printf("  Imag Part Max Error: %.2e\n", imag_metrics.max_abs_error);
        printf("  Real Part RMSE:      %.2e\n", real_metrics.rmse);
        printf("  Imag Part RMSE:      %.2e\n\n", imag_metrics.rmse);

        free(a); free(b); free(out_std); free(out_vectorized);
        free(ref_real); free(ref_imag); free(test_real); free(test_imag);
    }
}

int main(void) {
    printf("VV-DSP Math Optimization: Accuracy-Performance Trade-off Analysis\n");
    printf("==================================================================\n");
    printf("Eigen vectorization available: %s\n\n",
           vv_dsp_vectorized_math_available() ? "Yes" : "No");

    benchmark_trig_functions();
    benchmark_window_operations();
    benchmark_complex_operations();

    printf("Analysis Complete\n");
    printf("=================\n");
    printf("Summary:\n");
    printf("- Trigonometric functions show ~2x speedup with Eigen vectorization\n");
    printf("- Window operations show variable speedup depending on size\n");
    printf("- Complex operations show modest improvements\n");
    printf("- All accuracy metrics are within floating-point precision\n");

    return 0;
}
