/**
 * @file vv_dsp_bench.c
 * @brief Main benchmark executable for vv-dsp performance testing
 * @ingroup benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vv_dsp/vv_dsp.h"
#include "bench_framework.h"
#include "bench_timer.h"

/* Forward declarations for benchmark modules */
extern void run_stft_benchmarks(vv_bench_suite* suite);
extern void run_filter_benchmarks(vv_bench_suite* suite);
extern void run_resample_benchmarks(vv_bench_suite* suite);
extern void run_pipeline_benchmarks(vv_bench_suite* suite);
extern void run_denormal_benchmarks(vv_bench_suite* suite);

/* Global options */
static struct {
    int json_format;
    char* output_file;
    char* filter_pattern;
    int show_help;
    int list_benchmarks;
} options = {0, NULL, NULL, 0, 0};

static void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  --format=FORMAT     Output format: 'text' or 'json' (default: text)\n");
    printf("  --output=FILE       Output file (default: stdout)\n");
    printf("  --filter=PATTERN    Run only benchmarks matching pattern\n");
    printf("  --list              List available benchmarks and exit\n");
    printf("  --help              Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s                           # Run all benchmarks, text output\n", program_name);
    printf("  %s --format=json             # JSON output to stdout\n", program_name);
    printf("  %s --output=results.json     # Save results to file\n", program_name);
    printf("  %s --filter=stft             # Run only STFT benchmarks\n", program_name);
}

static void list_benchmarks(void) {
    printf("Available benchmark categories:\n");
    printf("  stft        - Short-Time Fourier Transform processing\n");
    printf("  filter      - FIR and IIR filtering performance\n");
    printf("  resample    - Audio resampling performance\n");
    printf("  pipeline    - End-to-end DSP pipeline performance\n");
    printf("  denormal    - Denormal number processing performance (FTZ/DAZ)\n");
    printf("\n");
    printf("Use --filter=CATEGORY to run specific benchmark categories.\n");
}

static int parse_arguments(int argc, char* argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (strcmp(arg, "--help") == 0) {
            options.show_help = 1;
            return 0;
        }
        else if (strcmp(arg, "--list") == 0) {
            options.list_benchmarks = 1;
            return 0;
        }
        else if (strncmp(arg, "--format=", 9) == 0) {
            const char* format = arg + 9;
            if (strcmp(format, "json") == 0) {
                options.json_format = 1;
            } else if (strcmp(format, "text") == 0) {
                options.json_format = 0;
            } else {
                fprintf(stderr, "Error: Unknown format '%s'. Use 'text' or 'json'.\n", format);
                return -1;
            }
        }
        else if (strncmp(arg, "--output=", 9) == 0) {
            options.output_file = (char*)(arg + 9);
        }
        else if (strncmp(arg, "--filter=", 9) == 0) {
            options.filter_pattern = (char*)(arg + 9);
        }
        else {
            fprintf(stderr, "Error: Unknown option '%s'\n", arg);
            return -1;
        }
    }

    return 0;
}

static int should_run_benchmark(const char* category) {
    if (!options.filter_pattern) {
        return 1;  /* Run all benchmarks if no filter */
    }

    return strstr(category, options.filter_pattern) != NULL;
}

int main(int argc, char* argv[]) {
    printf("DEBUG: Starting main function\n");

    /* Parse command line arguments */
    if (parse_arguments(argc, argv) != 0) {
        return 1;
    }
    printf("DEBUG: Arguments parsed\n");

    if (options.show_help) {
        print_usage(argv[0]);
        return 0;
    }

    if (options.list_benchmarks) {
        list_benchmarks();
        return 0;
    }

    printf("DEBUG: Before timer init\n");
    /* Initialize timing subsystem */
    if (vv_bench_timer_init() != 0) {
        fprintf(stderr, "Error: Failed to initialize high-resolution timer\n");
        return 1;
    }
    printf("DEBUG: Timer initialized\n");

    /* Open output file if specified */
    FILE* output_file = NULL;
    if (options.output_file) {
        output_file = fopen(options.output_file, "w");
        if (!output_file) {
            fprintf(stderr, "Error: Failed to open output file '%s'\n", options.output_file);
            return 1;
        }
    }
    printf("DEBUG: Output file handled\n");

    /* Initialize benchmark suite */
    vv_bench_suite suite;
    if (vv_bench_suite_init(&suite, output_file, options.json_format) != 0) {
        fprintf(stderr, "Error: Failed to initialize benchmark suite\n");
        if (output_file) fclose(output_file);
        return 1;
    }
    printf("DEBUG: Suite initialized\n");

    printf("DEBUG: Before should_run_benchmark check\n");
    /* Run benchmark categories */
    if (should_run_benchmark("stft")) {
        printf("DEBUG: About to call run_stft_benchmarks\n");
        run_stft_benchmarks(&suite);
        printf("DEBUG: run_stft_benchmarks returned\n");
    }

    /* Optional debug short-circuit: if VV_DSP_BENCH_ONLY_STFT env var is set, skip others */
    const char* only_stft = getenv("VV_DSP_BENCH_ONLY_STFT");
    if (only_stft && only_stft[0] != '\0') {
        printf("DEBUG: VV_DSP_BENCH_ONLY_STFT set -> skipping other benchmark categories\n");
    } else {
        if (should_run_benchmark("filter")) {
            run_filter_benchmarks(&suite);
        }

        if (should_run_benchmark("resample")) {
            run_resample_benchmarks(&suite);
        }

        if (should_run_benchmark("pipeline")) {
            run_pipeline_benchmarks(&suite);
        }

        if (should_run_benchmark("denormal")) {
            run_denormal_benchmarks(&suite);
        }
    }

    printf("DEBUG: Before write results\n");
    /* Write results */
    if (vv_bench_write_results(&suite) != 0) {
        fprintf(stderr, "Error: Failed to write benchmark results\n");
        if (output_file) fclose(output_file);
        return 1;
    }
    printf("DEBUG: Results written\n");

    /* Cleanup */
    if (output_file) {
        fclose(output_file);
    }

    if (!options.json_format && !options.output_file) {
        printf("\nBenchmark completed successfully.\n");
        if (options.filter_pattern) {
            printf("Filter pattern: %s\n", options.filter_pattern);
        }
    }

    printf("DEBUG: Main function completed\n");
    return 0;
}
