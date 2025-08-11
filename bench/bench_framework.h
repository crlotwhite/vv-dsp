/**
 * @file bench_framework.h
 * @brief Benchmark framework with JSON output support
 * @ingroup benchmark
 */

#ifndef VV_DSP_BENCH_FRAMEWORK_H
#define VV_DSP_BENCH_FRAMEWORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>
#include "bench_timer.h"

/**
 * @brief Maximum number of benchmark results
 */
#define VV_BENCH_MAX_RESULTS 100

/**
 * @brief Maximum length for benchmark names
 */
#define VV_BENCH_MAX_NAME_LEN 64

/**
 * @brief Benchmark result data structure
 */
typedef struct {
    char name[VV_BENCH_MAX_NAME_LEN];  ///< Name of the benchmark
    double elapsed_seconds;            ///< Elapsed time in seconds
    double samples_per_second;         ///< Throughput metric (if applicable)
    double real_time_factor;          ///< RTF for audio processing benchmarks
    size_t iterations;                 ///< Number of iterations performed
    int valid;                         ///< 1 if result is valid, 0 otherwise
} vv_bench_result;

/**
 * @brief Benchmark suite context
 */
typedef struct {
    vv_bench_result results[VV_BENCH_MAX_RESULTS];
    size_t num_results;
    FILE* output_file;                 ///< Output file for results (NULL for stdout)
    int json_format;                   ///< 1 for JSON output, 0 for text
} vv_bench_suite;

/**
 * @brief Initialize benchmark suite
 * @param suite Pointer to benchmark suite
 * @param output_file Output file (NULL for stdout)
 * @param json_format 1 for JSON format, 0 for text format
 * @return 0 on success, non-zero on error
 */
int vv_bench_suite_init(vv_bench_suite* suite, FILE* output_file, int json_format);

/**
 * @brief Add a benchmark result to the suite
 * @param suite Pointer to benchmark suite
 * @param name Name of the benchmark
 * @param elapsed_seconds Elapsed time in seconds
 * @param samples_per_second Throughput (samples per second, 0 if not applicable)
 * @param rtf Real-time factor (0 if not applicable)
 * @param iterations Number of iterations
 * @return 0 on success, non-zero on error
 */
int vv_bench_add_result(vv_bench_suite* suite, const char* name,
                        double elapsed_seconds, double samples_per_second,
                        double rtf, size_t iterations);

/**
 * @brief Write all benchmark results to output
 * @param suite Pointer to benchmark suite
 * @return 0 on success, non-zero on error
 */
int vv_bench_write_results(vv_bench_suite* suite);

/**
 * @brief Benchmark function signature
 */
typedef void (*vv_bench_func)(vv_bench_suite* suite);

/**
 * @brief Run a single benchmark with timing
 * @param suite Pointer to benchmark suite
 * @param name Name of the benchmark
 * @param func Function to benchmark
 * @param iterations Number of iterations to run
 * @param warmup_iterations Number of warmup iterations (not timed)
 * @return 0 on success, non-zero on error
 */
int vv_bench_run_timed(vv_bench_suite* suite, const char* name,
                       void (*func)(void), size_t iterations, size_t warmup_iterations);

#ifdef __cplusplus
}
#endif

#endif /* VV_DSP_BENCH_FRAMEWORK_H */
