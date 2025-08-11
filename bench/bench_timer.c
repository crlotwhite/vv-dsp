/**
 * @file bench_timer.c
 * @brief High-resolution timing utilities implementation
 * @ingroup benchmark
 */

#include "bench_timer.h"

/* Platform-specific includes and implementations */
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    static LARGE_INTEGER frequency;
    static int timer_initialized = 0;
#elif defined(__APPLE__)
    #include <mach/mach_time.h>
    static mach_timebase_info_data_t timebase;
    static int timer_initialized = 0;
#else
    /* Linux and other POSIX systems */
    #include <time.h>
    #include <errno.h>
    static int timer_initialized = 0;
#endif

int vv_bench_timer_init(void) {
    if (timer_initialized) return 0;

#ifdef _WIN32
    if (!QueryPerformanceFrequency(&frequency)) {
        return -1;  /* High-resolution timer not available */
    }
#elif defined(__APPLE__)
    if (mach_timebase_info(&timebase) != KERN_SUCCESS) {
        return -1;
    }
#else
    /* Linux: Check if CLOCK_MONOTONIC is available */
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return -1;
    }
#endif

    timer_initialized = 1;
    return 0;
}

vv_bench_time vv_bench_get_time(void) {
    vv_bench_time result = {0};

#ifdef _WIN32
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    result.ticks = (uint64_t)counter.QuadPart;
#elif defined(__APPLE__)
    result.ticks = mach_absolute_time();
#else
    /* Linux */
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        result.ticks = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }
#endif

    return result;
}

double vv_bench_elapsed_seconds(vv_bench_time start, vv_bench_time end) {
    if (end.ticks < start.ticks) {
        return 0.0;  /* Invalid time range */
    }

    uint64_t delta = end.ticks - start.ticks;

#ifdef _WIN32
    return (double)delta / (double)frequency.QuadPart;
#elif defined(__APPLE__)
    return (double)delta * (double)timebase.numer / ((double)timebase.denom * 1e9);
#else
    /* Linux: ticks are already in nanoseconds */
    return (double)delta / 1e9;
#endif
}

uint64_t vv_bench_elapsed_ns(vv_bench_time start, vv_bench_time end) {
    if (end.ticks < start.ticks) {
        return 0;  /* Invalid time range */
    }

    uint64_t delta = end.ticks - start.ticks;

#ifdef _WIN32
    /* Convert from performance counter ticks to nanoseconds */
    return (delta * 1000000000ULL) / (uint64_t)frequency.QuadPart;
#elif defined(__APPLE__)
    /* Convert from mach ticks to nanoseconds */
    return (delta * (uint64_t)timebase.numer) / (uint64_t)timebase.denom;
#else
    /* Linux: ticks are already in nanoseconds */
    return delta;
#endif
}
