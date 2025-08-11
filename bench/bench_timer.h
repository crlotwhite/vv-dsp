/**
 * @file bench_timer.h
 * @brief High-resolution timing utilities for benchmarking
 * @ingroup benchmark
 *
 * Provides cross-platform high-resolution timing functionality for accurate
 * performance measurements. Compatible with Windows (MSVC), Linux, and macOS.
 */

#ifndef VV_DSP_BENCH_TIMER_H
#define VV_DSP_BENCH_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief High-resolution timestamp type
 */
typedef struct {
    uint64_t ticks;  ///< Platform-specific high-resolution ticks
} vv_bench_time;

/**
 * @brief Get current high-resolution timestamp
 * @return Current timestamp
 *
 * @details Platform-specific implementation:
 * - Windows: QueryPerformanceCounter
 * - Linux: clock_gettime with CLOCK_MONOTONIC
 * - macOS: mach_absolute_time
 */
vv_bench_time vv_bench_get_time(void);

/**
 * @brief Calculate elapsed time in seconds between two timestamps
 * @param start Start timestamp
 * @param end End timestamp
 * @return Elapsed time in seconds (double precision)
 */
double vv_bench_elapsed_seconds(vv_bench_time start, vv_bench_time end);

/**
 * @brief Calculate elapsed time in nanoseconds between two timestamps
 * @param start Start timestamp
 * @param end End timestamp
 * @return Elapsed time in nanoseconds
 */
uint64_t vv_bench_elapsed_ns(vv_bench_time start, vv_bench_time end);

/**
 * @brief Initialize the timing subsystem
 * @return 0 on success, non-zero on error
 *
 * @details Must be called once before using timing functions.
 * Handles platform-specific initialization (e.g., QueryPerformanceFrequency on Windows).
 */
int vv_bench_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif /* VV_DSP_BENCH_TIMER_H */
