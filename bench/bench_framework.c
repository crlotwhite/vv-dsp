/**
 * @file bench_framework.c
 * @brief Benchmark framework implementation
 * @ingroup benchmark
 */

#include "bench_framework.h"
#include <string.h>
#include <stdlib.h>

int vv_bench_suite_init(vv_bench_suite* suite, FILE* output_file, int json_format) {
    if (!suite) return -1;

    memset(suite, 0, sizeof(vv_bench_suite));
    suite->output_file = output_file ? output_file : stdout;
    suite->json_format = json_format;
    suite->num_results = 0;

    return 0;
}

int vv_bench_add_result(vv_bench_suite* suite, const char* name,
                        double elapsed_seconds, double samples_per_second,
                        double rtf, size_t iterations) {
    if (!suite || !name || suite->num_results >= VV_BENCH_MAX_RESULTS) {
        return -1;
    }

    vv_bench_result* result = &suite->results[suite->num_results];

    /* Copy name with bounds checking */
    strncpy(result->name, name, VV_BENCH_MAX_NAME_LEN - 1);
    result->name[VV_BENCH_MAX_NAME_LEN - 1] = '\0';

    result->elapsed_seconds = elapsed_seconds;
    result->samples_per_second = samples_per_second;
    result->real_time_factor = rtf;
    result->iterations = iterations;
    result->valid = 1;

    suite->num_results++;
    return 0;
}

static void write_json_results(vv_bench_suite* suite) {
    size_t i;
    FILE* out = suite->output_file;

    fprintf(out, "{\n");
    fprintf(out, "  \"benchmark_suite\": \"vv-dsp\",\n");
    fprintf(out, "  \"results\": [\n");

    for (i = 0; i < suite->num_results; i++) {
        const vv_bench_result* r = &suite->results[i];
        if (!r->valid) continue;

        fprintf(out, "    {\n");
        fprintf(out, "      \"name\": \"%s\",\n", r->name);
        fprintf(out, "      \"elapsed_seconds\": %.9f,\n", r->elapsed_seconds);

        if (r->samples_per_second > 0.0) {
            fprintf(out, "      \"samples_per_second\": %.2f,\n", r->samples_per_second);
        }

        if (r->real_time_factor > 0.0) {
            fprintf(out, "      \"real_time_factor\": %.6f,\n", r->real_time_factor);
        }

        fprintf(out, "      \"iterations\": %zu\n", r->iterations);
        fprintf(out, "    }");

        if (i < suite->num_results - 1) {
            fprintf(out, ",");
        }
        fprintf(out, "\n");
    }

    fprintf(out, "  ]\n");
    fprintf(out, "}\n");
}

static void write_text_results(vv_bench_suite* suite) {
    size_t i;
    FILE* out = suite->output_file;

    fprintf(out, "vv-dsp Benchmark Results\n");
    fprintf(out, "========================\n\n");

    for (i = 0; i < suite->num_results; i++) {
        const vv_bench_result* r = &suite->results[i];
        if (!r->valid) continue;

        fprintf(out, "Benchmark: %s\n", r->name);
        fprintf(out, "  Elapsed time: %.9f seconds\n", r->elapsed_seconds);
        fprintf(out, "  Iterations: %zu\n", r->iterations);

        if (r->samples_per_second > 0.0) {
            fprintf(out, "  Throughput: %.2f samples/sec\n", r->samples_per_second);
        }

        if (r->real_time_factor > 0.0) {
            fprintf(out, "  Real-time factor: %.6f%s\n", r->real_time_factor,
                   (r->real_time_factor < 1.0) ? " (real-time capable)" : " (not real-time)");
        }

        fprintf(out, "\n");
    }
}

int vv_bench_write_results(vv_bench_suite* suite) {
    if (!suite) return -1;

    if (suite->json_format) {
        write_json_results(suite);
    } else {
        write_text_results(suite);
    }

    if (suite->output_file != stdout) {
        fflush(suite->output_file);
    }

    return 0;
}

int vv_bench_run_timed(vv_bench_suite* suite, const char* name,
                       void (*func)(void), size_t iterations, size_t warmup_iterations) {
    if (!suite || !name || !func || iterations == 0) {
        return -1;
    }

    size_t i;

    /* Warmup iterations */
    for (i = 0; i < warmup_iterations; i++) {
        func();
    }

    /* Timed iterations */
    vv_bench_time start = vv_bench_get_time();

    for (i = 0; i < iterations; i++) {
        func();
    }

    vv_bench_time end = vv_bench_get_time();
    double elapsed = vv_bench_elapsed_seconds(start, end);

    return vv_bench_add_result(suite, name, elapsed, 0.0, 0.0, iterations);
}
