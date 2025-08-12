#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>  // For alarm() function
#include "bench_framework.h"

// Forward declaration
extern void run_stft_benchmarks(vv_bench_suite* suite);

int main(void) {
    printf("Starting simple benchmark test...\n");

    // Initialize suite
    vv_bench_suite suite;
    if (vv_bench_suite_init(&suite, NULL, 0) != 0) {
        printf("Failed to initialize suite\n");
        return 1;
    }

    printf("Suite initialized, calling STFT benchmarks...\n");

    // Set alarm to interrupt infinite loop
    alarm(5);  // 5 second timeout

    run_stft_benchmarks(&suite);

    printf("STFT benchmarks completed\n");

    if (vv_bench_write_results(&suite) != 0) {
        printf("Failed to write results\n");
        return 1;
    }

    printf("All done!\n");
    return 0;
}
