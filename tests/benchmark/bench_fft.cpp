/**
 * @file bench_fft.cpp
 * @brief Google Benchmark suite for vv-dsp FFT functions
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <cmath>
#include <complex>
#include <random>

extern "C" {
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/vv_dsp_types.h"
}

class FFTBenchmarkFixture : public benchmark::Fixture {
protected:
    std::mt19937 gen{42}; // Fixed seed for reproducibility
    std::uniform_real_distribution<vv_dsp_real> dist{-1.0, 1.0};

    void generateRandomComplex(std::vector<vv_dsp_cpx>& data) {
        for (auto& sample : data) {
            sample.re = dist(gen);
            sample.im = dist(gen);
        }
    }

    void generateRandomReal(std::vector<vv_dsp_real>& data) {
        for (auto& sample : data) {
            sample = dist(gen);
        }
    }
};

// Complex FFT Forward Transform Benchmark
BENCHMARK_DEFINE_F(FFTBenchmarkFixture, ComplexFFTForward)(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_cpx> input(N), output(N);
    generateRandomComplex(input);

    vv_dsp_fft_plan* plan = nullptr;
    if (vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan) != VV_DSP_OK) {
        state.SkipWithError("Failed to create FFT plan");
        return;
    }

    for (auto _ : state) {
        vv_dsp_fft_execute(plan, input.data(), output.data());
        benchmark::DoNotOptimize(output.data());
    }

    vv_dsp_fft_destroy(plan);
    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
    state.SetBytesProcessed(state.iterations() * N * sizeof(vv_dsp_cpx) * 2); // input + output
}

// Complex FFT Inverse Transform Benchmark
BENCHMARK_DEFINE_F(FFTBenchmarkFixture, ComplexFFTInverse)(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_cpx> input(N), output(N);
    generateRandomComplex(input);

    vv_dsp_fft_plan* plan = nullptr;
    if (vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &plan) != VV_DSP_OK) {
        state.SkipWithError("Failed to create FFT plan");
        return;
    }

    for (auto _ : state) {
        vv_dsp_fft_execute(plan, input.data(), output.data());
        benchmark::DoNotOptimize(output.data());
    }

    vv_dsp_fft_destroy(plan);
    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
    state.SetBytesProcessed(state.iterations() * N * sizeof(vv_dsp_cpx) * 2);
}

// Real FFT Forward Transform Benchmark (R2C)
BENCHMARK_DEFINE_F(FFTBenchmarkFixture, RealFFTForward)(benchmark::State& state) {
    const size_t N = state.range(0);
    const size_t spectrum_size = N / 2 + 1;
    std::vector<vv_dsp_real> input(N);
    std::vector<vv_dsp_cpx> output(spectrum_size);
    generateRandomReal(input);

    vv_dsp_fft_plan* plan = nullptr;
    if (vv_dsp_fft_make_plan(N, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &plan) != VV_DSP_OK) {
        state.SkipWithError("Failed to create real FFT plan");
        return;
    }

    for (auto _ : state) {
        vv_dsp_fft_execute(plan, input.data(), output.data());
        benchmark::DoNotOptimize(output.data());
    }

    vv_dsp_fft_destroy(plan);
    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
    state.SetBytesProcessed(state.iterations() * (N * sizeof(vv_dsp_real) + spectrum_size * sizeof(vv_dsp_cpx)));
}

// Real FFT Inverse Transform Benchmark (C2R)
BENCHMARK_DEFINE_F(FFTBenchmarkFixture, RealFFTInverse)(benchmark::State& state) {
    const size_t N = state.range(0);
    const size_t spectrum_size = N / 2 + 1;
    std::vector<vv_dsp_cpx> input(spectrum_size);
    std::vector<vv_dsp_real> output(N);
    generateRandomComplex(input);

    vv_dsp_fft_plan* plan = nullptr;
    if (vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2R, VV_DSP_FFT_BACKWARD, &plan) != VV_DSP_OK) {
        state.SkipWithError("Failed to create real IFFT plan");
        return;
    }

    for (auto _ : state) {
        vv_dsp_fft_execute(plan, input.data(), output.data());
        benchmark::DoNotOptimize(output.data());
    }

    vv_dsp_fft_destroy(plan);
    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations() * N);
    state.SetBytesProcessed(state.iterations() * (spectrum_size * sizeof(vv_dsp_cpx) + N * sizeof(vv_dsp_real)));
}

// FFT Plan Creation Benchmark
BENCHMARK_DEFINE_F(FFTBenchmarkFixture, FFTPlanCreation)(benchmark::State& state) {
    const size_t N = state.range(0);

    for (auto _ : state) {
        vv_dsp_fft_plan* plan = nullptr;
        vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);
        benchmark::DoNotOptimize(plan);
        vv_dsp_fft_destroy(plan);
    }

    state.SetComplexityN(N);
    state.SetItemsProcessed(state.iterations());
}

// Register benchmarks with power-of-2 sizes
BENCHMARK_REGISTER_F(FFTBenchmarkFixture, ComplexFFTForward)
    ->RangeMultiplier(2)->Range(64, 8192)->Complexity(benchmark::oNLogN);

BENCHMARK_REGISTER_F(FFTBenchmarkFixture, ComplexFFTInverse)
    ->RangeMultiplier(2)->Range(64, 8192)->Complexity(benchmark::oNLogN);

BENCHMARK_REGISTER_F(FFTBenchmarkFixture, RealFFTForward)
    ->RangeMultiplier(2)->Range(64, 8192)->Complexity(benchmark::oNLogN);

BENCHMARK_REGISTER_F(FFTBenchmarkFixture, RealFFTInverse)
    ->RangeMultiplier(2)->Range(64, 8192)->Complexity(benchmark::oNLogN);

BENCHMARK_REGISTER_F(FFTBenchmarkFixture, FFTPlanCreation)
    ->RangeMultiplier(2)->Range(64, 8192)->Complexity();

// Non-power-of-2 FFT benchmarks
BENCHMARK_REGISTER_F(FFTBenchmarkFixture, ComplexFFTForward)
    ->Args({100})->Args({200})->Args({300})->Args({500})->Args({1000})
    ->Args({1500})->Args({3000})->Args({5000})
    ->Name("ComplexFFTForward_NonPow2");

BENCHMARK_REGISTER_F(FFTBenchmarkFixture, RealFFTForward)
    ->Args({100})->Args({200})->Args({300})->Args({500})->Args({1000})
    ->Args({1500})->Args({3000})->Args({5000})
    ->Name("RealFFTForward_NonPow2");

// Backend-specific benchmarks (if multiple backends are available)
static void BM_FFTBackendComparison(benchmark::State& state) {
    const size_t N = 1024;
    std::vector<vv_dsp_cpx> input(N), output(N);

    // Generate test data
    std::mt19937 gen{42};
    std::uniform_real_distribution<vv_dsp_real> dist{-1.0, 1.0};
    for (auto& sample : input) {
        sample.re = dist(gen);
        sample.im = dist(gen);
    }

    // Get current backend
    vv_dsp_fft_backend current_backend = vv_dsp_fft_get_backend();

    vv_dsp_fft_plan* plan = nullptr;
    if (vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan) != VV_DSP_OK) {
        state.SkipWithError("Failed to create FFT plan");
        return;
    }

    for (auto _ : state) {
        vv_dsp_fft_execute(plan, input.data(), output.data());
        benchmark::DoNotOptimize(output.data());
    }

    vv_dsp_fft_destroy(plan);
    state.SetItemsProcessed(state.iterations() * N);

    // Set label with backend name
    std::string backend_name;
    switch (current_backend) {
        case VV_DSP_FFT_BACKEND_KISS: backend_name = "Kiss"; break;
        case VV_DSP_FFT_BACKEND_FFTW: backend_name = "FFTW"; break;
        case VV_DSP_FFT_BACKEND_FFTS: backend_name = "FFTS"; break;
        default: backend_name = "Unknown"; break;
    }
    state.SetLabel(backend_name);
}

BENCHMARK(BM_FFTBackendComparison);

// Memory throughput benchmark
static void BM_FFTMemoryThroughput(benchmark::State& state) {
    const size_t N = state.range(0);
    std::vector<vv_dsp_cpx> input(N), output(N);

    // Generate test data
    std::mt19937 gen{42};
    std::uniform_real_distribution<vv_dsp_real> dist{-1.0, 1.0};
    for (auto& sample : input) {
        sample.re = dist(gen);
        sample.im = dist(gen);
    }

    vv_dsp_fft_plan* plan = nullptr;
    if (vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan) != VV_DSP_OK) {
        state.SkipWithError("Failed to create FFT plan");
        return;
    }

    for (auto _ : state) {
        vv_dsp_fft_execute(plan, input.data(), output.data());
        benchmark::DoNotOptimize(output.data());
    }

    vv_dsp_fft_destroy(plan);

    // Calculate memory throughput
    size_t bytes_per_iteration = N * sizeof(vv_dsp_cpx) * 2; // input + output
    state.SetBytesProcessed(state.iterations() * bytes_per_iteration);
    state.SetItemsProcessed(state.iterations() * N);
}

BENCHMARK(BM_FFTMemoryThroughput)->RangeMultiplier(2)->Range(64, 8192);
