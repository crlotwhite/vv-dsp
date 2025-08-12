/**
 * @file test_simd_core.c
 * @brief Test SIMD-optimized core functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "vv_dsp/core/simd_core.h"
#include "vv_dsp/core/simd_utils.h"

/* External declaration for simd features string */
extern const char* vv_dsp_simd_features_string(void);

#define TEST_SIZE 1000
#define TOLERANCE 1e-6f

/* Helper function to generate test data */
static void generate_test_data(float* data, size_t n) {
    srand(42); /* Fixed seed for reproducible tests */
    for (size_t i = 0; i < n; i++) {
        data[i] = (float)(rand() % 1000) / 100.0f - 5.0f; /* Range: -5.0 to 5.0 */
    }
}

/* Helper function to compare floats with tolerance */
static int float_equals(float a, float b, float tolerance) {
    return fabsf(a - b) < tolerance;
}

/* Test element-wise addition */
static int test_add_real_simd(void) {
    printf("Testing vv_dsp_add_real_simd...\n");

    float* a = malloc(TEST_SIZE * sizeof(float));
    float* b = malloc(TEST_SIZE * sizeof(float));
    float* result = malloc(TEST_SIZE * sizeof(float));
    float* expected = malloc(TEST_SIZE * sizeof(float));

    if (!a || !b || !result || !expected) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(a, TEST_SIZE);
    generate_test_data(b, TEST_SIZE);

    /* Calculate expected result (scalar) */
    for (size_t i = 0; i < TEST_SIZE; i++) {
        expected[i] = a[i] + b[i];
    }

    /* Calculate SIMD result */
    vv_dsp_status status = vv_dsp_add_real_simd(a, b, result, TEST_SIZE);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        goto cleanup;
    }

    /* Compare results */
    int passed = 1;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        if (!float_equals(result[i], expected[i], TOLERANCE)) {
            printf("  FAILED: Mismatch at index %zu: got %f, expected %f\n",
                   i, result[i], expected[i]);
            passed = 0;
            break;
        }
    }

    if (passed) {
        printf("  PASSED\n");
    }

cleanup:
    free(a);
    free(b);
    free(result);
    free(expected);
    return passed;
}

/* Test element-wise multiplication */
static int test_mul_real_simd(void) {
    printf("Testing vv_dsp_mul_real_simd...\n");

    float* a = malloc(TEST_SIZE * sizeof(float));
    float* b = malloc(TEST_SIZE * sizeof(float));
    float* result = malloc(TEST_SIZE * sizeof(float));
    float* expected = malloc(TEST_SIZE * sizeof(float));

    if (!a || !b || !result || !expected) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(a, TEST_SIZE);
    generate_test_data(b, TEST_SIZE);

    /* Calculate expected result (scalar) */
    for (size_t i = 0; i < TEST_SIZE; i++) {
        expected[i] = a[i] * b[i];
    }

    /* Calculate SIMD result */
    vv_dsp_status status = vv_dsp_mul_real_simd(a, b, result, TEST_SIZE);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        goto cleanup;
    }

    /* Compare results */
    int passed = 1;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        if (!float_equals(result[i], expected[i], TOLERANCE)) {
            printf("  FAILED: Mismatch at index %zu: got %f, expected %f\n",
                   i, result[i], expected[i]);
            passed = 0;
            break;
        }
    }

    if (passed) {
        printf("  PASSED\n");
    }

cleanup:
    free(a);
    free(b);
    free(result);
    free(expected);
    return passed;
}

/* Test sum function */
static int test_sum_optimized(void) {
    printf("Testing vv_dsp_sum_optimized...\n");

    float* data = malloc(TEST_SIZE * sizeof(float));
    if (!data) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(data, TEST_SIZE);

    /* Calculate expected result (scalar with Kahan summation) */
    double expected_sum = 0.0, c = 0.0;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        double y = (double)data[i] - c;
        double t = expected_sum + y;
        c = (t - expected_sum) - y;
        expected_sum = t;
    }

    /* Calculate SIMD result */
    float result_sum;
    vv_dsp_status status = vv_dsp_sum_optimized(data, TEST_SIZE, &result_sum);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        free(data);
        return 0;
    }

    /* Compare results */
    int passed = float_equals(result_sum, (float)expected_sum, TOLERANCE * 100); /* Increased tolerance for SIMD vs scalar differences */

    if (passed) {
        printf("  PASSED (sum: %f)\n", result_sum);
    } else {
        printf("  FAILED: got %f, expected %f\n", result_sum, (float)expected_sum);
    }

    free(data);
    return passed;
}

/* Test RMS function */
static int test_rms_optimized(void) {
    printf("Testing vv_dsp_rms_optimized...\n");

    float* data = malloc(TEST_SIZE * sizeof(float));
    if (!data) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(data, TEST_SIZE);

    /* Calculate expected result (scalar) */
    double acc = 0.0;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        double v = (double)data[i];
        acc += v * v;
    }
    float expected_rms = (float)sqrt(acc / (double)TEST_SIZE);

    /* Calculate SIMD result */
    float result_rms;
    vv_dsp_status status = vv_dsp_rms_optimized(data, TEST_SIZE, &result_rms);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        free(data);
        return 0;
    }

    /* Compare results */
    int passed = float_equals(result_rms, expected_rms, TOLERANCE * 10);

    if (passed) {
        printf("  PASSED (RMS: %f)\n", result_rms);
    } else {
        printf("  FAILED: got %f, expected %f\n", result_rms, expected_rms);
    }

    free(data);
    return passed;
}

/* Test peak finding function */
static int test_peak_optimized(void) {
    printf("Testing vv_dsp_peak_optimized...\n");

    float* data = malloc(TEST_SIZE * sizeof(float));
    if (!data) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(data, TEST_SIZE);

    /* Calculate expected result (scalar) */
    float expected_min = data[0];
    float expected_max = data[0];
    for (size_t i = 1; i < TEST_SIZE; i++) {
        if (data[i] < expected_min) expected_min = data[i];
        if (data[i] > expected_max) expected_max = data[i];
    }

    /* Calculate SIMD result */
    float result_min, result_max;
    vv_dsp_status status = vv_dsp_peak_optimized(data, TEST_SIZE, &result_min, &result_max);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        free(data);
        return 0;
    }

    /* Compare results */
    int passed = float_equals(result_min, expected_min, TOLERANCE) &&
                 float_equals(result_max, expected_max, TOLERANCE);

    if (passed) {
        printf("  PASSED (min: %f, max: %f)\n", result_min, result_max);
    } else {
        printf("  FAILED: got min=%f max=%f, expected min=%f max=%f\n",
               result_min, result_max, expected_min, expected_max);
    }

    free(data);
    return passed;
}

/* Test mean function */
static int test_mean_optimized(void) {
    printf("Testing vv_dsp_mean_optimized...\n");

    float* data = malloc(TEST_SIZE * sizeof(float));
    if (!data) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(data, TEST_SIZE);

    /* Calculate expected result (scalar) */
    double expected_sum = 0.0;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        expected_sum += (double)data[i];
    }
    float expected_mean = (float)(expected_sum / (double)TEST_SIZE);

    /* Calculate SIMD result */
    float result_mean;
    vv_dsp_status status = vv_dsp_mean_optimized(data, TEST_SIZE, &result_mean);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        free(data);
        return 0;
    }

    /* Compare results */
    int passed = float_equals(result_mean, expected_mean, TOLERANCE * 100); /* Increased tolerance for SIMD vs scalar differences */

    if (passed) {
        printf("  PASSED (mean: %f)\n", result_mean);
    } else {
        printf("  FAILED: got %f, expected %f\n", result_mean, expected_mean);
    }

    free(data);
    return passed;
}

/* Test variance function */
static int test_variance_optimized(void) {
    printf("Testing vv_dsp_variance_optimized...\n");

    float* data = malloc(TEST_SIZE * sizeof(float));
    if (!data) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(data, TEST_SIZE);

    /* Calculate expected result (scalar two-pass) */
    double sum = 0.0;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        sum += (double)data[i];
    }
    double mean = sum / (double)TEST_SIZE;

    double var_sum = 0.0;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        double diff = (double)data[i] - mean;
        var_sum += diff * diff;
    }
    float expected_variance = (float)(var_sum / (double)(TEST_SIZE - 1)); /* unbiased */

    /* Calculate SIMD result */
    float result_variance;
    vv_dsp_status status = vv_dsp_variance_optimized(data, TEST_SIZE, &result_variance);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        free(data);
        return 0;
    }

    /* Compare results */
    int passed = float_equals(result_variance, expected_variance, TOLERANCE * 100); /* Larger tolerance for variance */

    if (passed) {
        printf("  PASSED (variance: %f)\n", result_variance);
    } else {
        printf("  FAILED: got %f, expected %f\n", result_variance, expected_variance);
    }

    free(data);
    return passed;
}

/* Test standard deviation function */
static int test_stddev_optimized(void) {
    printf("Testing vv_dsp_stddev_optimized...\n");

    float* data = malloc(TEST_SIZE * sizeof(float));
    if (!data) {
        printf("Memory allocation failed\n");
        return 0;
    }

    generate_test_data(data, TEST_SIZE);

    /* Calculate expected result (scalar two-pass) */
    double sum = 0.0;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        sum += (double)data[i];
    }
    double mean = sum / (double)TEST_SIZE;

    double var_sum = 0.0;
    for (size_t i = 0; i < TEST_SIZE; i++) {
        double diff = (double)data[i] - mean;
        var_sum += diff * diff;
    }
    double variance = var_sum / (double)(TEST_SIZE - 1); /* unbiased */
    float expected_stddev = (float)sqrt(variance);

    /* Calculate SIMD result */
    float result_stddev;
    vv_dsp_status status = vv_dsp_stddev_optimized(data, TEST_SIZE, &result_stddev);

    if (status != VV_DSP_OK) {
        printf("  FAILED: Function returned error status\n");
        free(data);
        return 0;
    }

    /* Compare results */
    int passed = float_equals(result_stddev, expected_stddev, TOLERANCE * 100);

    if (passed) {
        printf("  PASSED (stddev: %f)\n", result_stddev);
    } else {
        printf("  FAILED: got %f, expected %f\n", result_stddev, expected_stddev);
    }

    free(data);
    return passed;
}
static void benchmark_simd_functions(void) {
    printf("\nPerformance Benchmark:\n");

    const size_t bench_size = 100000;
    const int iterations = 1000;

    float* a = malloc(bench_size * sizeof(float));
    float* b = malloc(bench_size * sizeof(float));
    float* result = malloc(bench_size * sizeof(float));

    if (!a || !b || !result) {
        printf("Memory allocation failed for benchmark\n");
        return;
    }

    generate_test_data(a, bench_size);
    generate_test_data(b, bench_size);

    /* Benchmark add operation */
    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        vv_dsp_add_real_simd(a, b, result, bench_size);
    }
    clock_t end = clock();

    double add_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Add (SIMD): %f seconds for %d iterations\n", add_time, iterations);

    /* Benchmark scalar add for comparison */
    start = clock();
    for (int i = 0; i < iterations; i++) {
        for (size_t j = 0; j < bench_size; j++) {
            result[j] = a[j] + b[j];
        }
    }
    end = clock();

    double scalar_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Add (scalar): %f seconds for %d iterations\n", scalar_time, iterations);
    printf("  Speedup: %.2fx\n", scalar_time / add_time);

    free(a);
    free(b);
    free(result);
}

int main(void) {
    printf("=== SIMD Core Functions Test ===\n");
    printf("Testing SIMD-optimized core functions\n");
    printf("\n");

    int total_tests = 0;
    int passed_tests = 0;

    /* Run all tests */
    total_tests++; passed_tests += test_add_real_simd();
    total_tests++; passed_tests += test_mul_real_simd();
    total_tests++; passed_tests += test_sum_optimized();
    total_tests++; passed_tests += test_rms_optimized();
    total_tests++; passed_tests += test_peak_optimized();
    total_tests++; passed_tests += test_mean_optimized();
    total_tests++; passed_tests += test_variance_optimized();
    total_tests++; passed_tests += test_stddev_optimized();

    printf("\nTest Results: %d/%d passed\n", passed_tests, total_tests);

    if (passed_tests == total_tests) {
        benchmark_simd_functions();
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
