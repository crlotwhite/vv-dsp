/**
 * @file bench_window.cpp
 * @brief Google Benchmark suite for vv-dsp window functions
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <cmath>

extern "C" {
#include "vv_dsp/window.h"
#include "vv_dsp/vv_dsp_types.h"
}

// Benchmark window function performance across different sizes
static void BM_WindowHann(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_hann(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }

    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_WindowHamming(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_hamming(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }

    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_WindowBlackman(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_blackman(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }

    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_WindowBoxcar(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_boxcar(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }

    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_WindowKaiser(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_real> window(N);
    const vv_dsp_real beta = 8.0; // Standard Kaiser parameter

    for (auto _ : state) {
        vv_dsp_window_kaiser(N, beta, window.data());
        benchmark::DoNotOptimize(window.data());
    }

    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
}

// Register benchmarks with range of window sizes
BENCHMARK(BM_WindowHann)->Range(64, 8192)->Complexity();
BENCHMARK(BM_WindowHamming)->Range(64, 8192)->Complexity();
BENCHMARK(BM_WindowBlackman)->Range(64, 8192)->Complexity();
BENCHMARK(BM_WindowBoxcar)->Range(64, 8192)->Complexity();
BENCHMARK(BM_WindowKaiser)->Range(64, 8192)->Complexity();

// Benchmark comparing different window types at fixed size
static void BM_WindowComparison_Hann(benchmark::State& state) {
    const size_t N = 1024;
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_hann(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }
}

static void BM_WindowComparison_Hamming(benchmark::State& state) {
    const size_t N = 1024;
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_hamming(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }
}

static void BM_WindowComparison_Blackman(benchmark::State& state) {
    const size_t N = 1024;
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_blackman(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }
}

static void BM_WindowComparison_Boxcar(benchmark::State& state) {
    const size_t N = 1024;
    std::vector<vv_dsp_real> window(N);

    for (auto _ : state) {
        vv_dsp_window_boxcar(N, window.data());
        benchmark::DoNotOptimize(window.data());
    }
}

static void BM_WindowComparison_Kaiser(benchmark::State& state) {
    const size_t N = 1024;
    std::vector<vv_dsp_real> window(N);
    const vv_dsp_real beta = 8.0;

    for (auto _ : state) {
        vv_dsp_window_kaiser(N, beta, window.data());
        benchmark::DoNotOptimize(window.data());
    }
}

// Register comparison benchmarks
BENCHMARK(BM_WindowComparison_Hann);
BENCHMARK(BM_WindowComparison_Hamming);
BENCHMARK(BM_WindowComparison_Blackman);
BENCHMARK(BM_WindowComparison_Boxcar);
BENCHMARK(BM_WindowComparison_Kaiser);
