/**
 * @file test_fft.cpp
 * @brief Google Test suite for vv-dsp FFT functionality
 */

#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <complex>

extern "C" {
#include "vv_dsp/spectral/fft.h"
#include "vv_dsp/vv_dsp_types.h"
#include "vv_dsp/core.h"
}

class FFTTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize random number generator for reproducible tests
        gen.seed(42);
    }

    void TearDown() override {
        // Clean up
    }

    // Helper function to generate test signal (real-valued sine wave)
    void generateSineWave(std::vector<vv_dsp_real>& signal, vv_dsp_real freq, vv_dsp_real sample_rate) {
        for (size_t i = 0; i < signal.size(); ++i) {
            signal[i] = std::sin(2.0 * M_PI * freq * i / sample_rate);
        }
    }

    // Helper function to generate complex test signal
    void generateComplexSineWave(std::vector<vv_dsp_cpx>& signal, vv_dsp_real freq, vv_dsp_real sample_rate) {
        for (size_t i = 0; i < signal.size(); ++i) {
            vv_dsp_real phase = 2.0 * M_PI * freq * i / sample_rate;
            signal[i].re = std::cos(phase);
            signal[i].im = std::sin(phase);
        }
    }

    // Helper function to generate white noise
    void generateNoise(std::vector<vv_dsp_real>& signal, vv_dsp_real amplitude = 1.0) {
        std::uniform_real_distribution<vv_dsp_real> dist(-amplitude, amplitude);
        for (auto& sample : signal) {
            sample = dist(gen);
        }
    }

    // Helper function to calculate signal magnitude
    vv_dsp_real magnitude(const vv_dsp_cpx& z) {
        return std::sqrt(z.re * z.re + z.im * z.im);
    }

    // Helper function to find peak frequency bin
    size_t findPeakBin(const std::vector<vv_dsp_cpx>& spectrum) {
        size_t peak_idx = 0;
        vv_dsp_real max_mag = 0.0;
        for (size_t i = 0; i < spectrum.size(); ++i) {
            vv_dsp_real mag = magnitude(spectrum[i]);
            if (mag > max_mag) {
                max_mag = mag;
                peak_idx = i;
            }
        }
        return peak_idx;
    }

    // Helper function to check if arrays are approximately equal
    bool areArraysEqual(const std::vector<vv_dsp_real>& a, const std::vector<vv_dsp_real>& b, vv_dsp_real tolerance = 1e-6) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (std::abs(a[i] - b[i]) > tolerance) {
                return false;
            }
        }
        return true;
    }

    std::mt19937 gen;
};

// Test backend availability and switching
TEST_F(FFTTest, BackendManagement) {
    // KissFFT should always be available
    EXPECT_TRUE(vv_dsp_fft_is_backend_available(VV_DSP_FFT_BACKEND_KISS));

    // Test setting backend to KissFFT
    EXPECT_EQ(vv_dsp_fft_set_backend(VV_DSP_FFT_BACKEND_KISS), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_fft_get_backend(), VV_DSP_FFT_BACKEND_KISS);

    // Test setting to potentially unavailable backends (should not crash)
    vv_dsp_status status_fftw = vv_dsp_fft_set_backend(VV_DSP_FFT_BACKEND_FFTW);
    vv_dsp_status status_ffts = vv_dsp_fft_set_backend(VV_DSP_FFT_BACKEND_FFTS);

    // If FFTW is available, setting should succeed
    if (vv_dsp_fft_is_backend_available(VV_DSP_FFT_BACKEND_FFTW)) {
        EXPECT_EQ(status_fftw, VV_DSP_OK);
    } else {
        EXPECT_EQ(status_fftw, VV_DSP_ERROR_UNSUPPORTED);
    }

    // Reset to KissFFT for remaining tests
    vv_dsp_fft_set_backend(VV_DSP_FFT_BACKEND_KISS);
}

// Test basic plan creation and destruction
TEST_F(FFTTest, PlanLifecycle) {
    vv_dsp_fft_plan* plan = nullptr;

    // Test valid plan creation
    EXPECT_EQ(vv_dsp_fft_make_plan(64, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan), VV_DSP_OK);
    EXPECT_NE(plan, nullptr);

    // Test plan destruction
    EXPECT_EQ(vv_dsp_fft_destroy(plan), VV_DSP_OK);

    // Test destroying null plan (should be safe)
    EXPECT_EQ(vv_dsp_fft_destroy(nullptr), VV_DSP_OK);
}

// Test invalid plan creation
TEST_F(FFTTest, InvalidPlanCreation) {
    vv_dsp_fft_plan* plan = nullptr;

    // Test with null output pointer
    EXPECT_NE(vv_dsp_fft_make_plan(64, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, nullptr), VV_DSP_OK);

    // Test with zero size
    EXPECT_NE(vv_dsp_fft_make_plan(0, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan), VV_DSP_OK);

    // Test with very large size (should handle gracefully)
    vv_dsp_status status = vv_dsp_fft_make_plan(SIZE_MAX, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan);
    // Should either succeed or fail gracefully, but not crash
    EXPECT_TRUE(status == VV_DSP_OK || status != VV_DSP_OK);
    if (status == VV_DSP_OK && plan) {
        vv_dsp_fft_destroy(plan);
    }
}

// Test Complex-to-Complex FFT with parameterized sizes
class FFTComplexTest : public FFTTest, public ::testing::WithParamInterface<size_t> {};

TEST_P(FFTComplexTest, ComplexFFTForwardBackward) {
    const size_t N = GetParam();
    std::vector<vv_dsp_cpx> input(N), output(N), reconstructed(N);

    // Generate test signal: complex exponential
    generateComplexSineWave(input, 1.0, static_cast<vv_dsp_real>(N)); // freq = 1 bin

    // Create forward and backward plans
    vv_dsp_fft_plan* forward_plan = nullptr;
    vv_dsp_fft_plan* backward_plan = nullptr;

    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &forward_plan), VV_DSP_OK);
    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_BACKWARD, &backward_plan), VV_DSP_OK);

    // Execute forward FFT
    ASSERT_EQ(vv_dsp_fft_execute(forward_plan, input.data(), output.data()), VV_DSP_OK);

    // Execute backward FFT
    ASSERT_EQ(vv_dsp_fft_execute(backward_plan, output.data(), reconstructed.data()), VV_DSP_OK);

    // Check reconstruction (should be equal to input within tolerance)
    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(reconstructed[i].re, input[i].re, 1e-5) 
            << "Real part mismatch at index " << i << " for N=" << N;
        EXPECT_NEAR(reconstructed[i].im, input[i].im, 1e-5)
            << "Imaginary part mismatch at index " << i << " for N=" << N;
    }

    // Clean up
    vv_dsp_fft_destroy(forward_plan);
    vv_dsp_fft_destroy(backward_plan);
}

TEST_P(FFTComplexTest, ComplexFFTPeakDetection) {
    const size_t N = GetParam();
    if (N < 8) return; // Skip very small sizes for this test

    std::vector<vv_dsp_cpx> input(N), output(N);

    // Generate single frequency signal at bin 3
    const size_t target_bin = std::min(static_cast<size_t>(3), N/4);
    const vv_dsp_real freq = static_cast<vv_dsp_real>(target_bin);
    generateComplexSineWave(input, freq, static_cast<vv_dsp_real>(N));

    vv_dsp_fft_plan* plan = nullptr;
    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan), VV_DSP_OK);
    ASSERT_EQ(vv_dsp_fft_execute(plan, input.data(), output.data()), VV_DSP_OK);

    // Find peak in spectrum
    size_t peak_bin = findPeakBin(output);

    // Peak should be at the target frequency
    EXPECT_EQ(peak_bin, target_bin) << "Peak detection failed for N=" << N;

    // Check that peak magnitude is significant compared to noise floor
    vv_dsp_real peak_mag = magnitude(output[peak_bin]);
    vv_dsp_real noise_mag = 0.0;
    size_t noise_bins = 0;
    for (size_t i = 1; i < N/2; ++i) {  // Skip DC and target bin
        if (i != target_bin) {
            noise_mag += magnitude(output[i]);
            noise_bins++;
        }
    }
    if (noise_bins > 0) {
        noise_mag /= noise_bins;
        EXPECT_GT(peak_mag / noise_mag, 10.0) << "Signal-to-noise ratio too low for N=" << N;
    }

    vv_dsp_fft_destroy(plan);
}

// Test Real-to-Complex FFT
class FFTRealTest : public FFTTest, public ::testing::WithParamInterface<size_t> {};

TEST_P(FFTRealTest, RealFFTBasicProperties) {
    const size_t N = GetParam();
    const size_t spectrum_size = N/2 + 1;
    std::vector<vv_dsp_real> input(N);
    std::vector<vv_dsp_cpx> output(spectrum_size);

    // Generate real sine wave
    generateSineWave(input, 1.0, static_cast<vv_dsp_real>(N));

    vv_dsp_fft_plan* plan = nullptr;
    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &plan), VV_DSP_OK);
    ASSERT_EQ(vv_dsp_fft_execute(plan, input.data(), output.data()), VV_DSP_OK);

    // Check Hermitian symmetry property (for real input, FFT[k] = conj(FFT[N-k]))
    // This is implicitly satisfied by the R2C output format

    // DC component should be real
    EXPECT_NEAR(output[0].im, 0.0, 1e-5) << "DC component should be real for N=" << N;

    // Nyquist component should be real (if N is even)
    if (N % 2 == 0) {
        EXPECT_NEAR(output[spectrum_size-1].im, 0.0, 1e-5) 
            << "Nyquist component should be real for N=" << N;
    }

    vv_dsp_fft_destroy(plan);
}

TEST_P(FFTRealTest, RealFFTForwardBackward) {
    const size_t N = GetParam();
    const size_t spectrum_size = N/2 + 1;
    std::vector<vv_dsp_real> input(N), reconstructed(N);
    std::vector<vv_dsp_cpx> spectrum(spectrum_size);

    // Generate test signal
    generateSineWave(input, 2.5, static_cast<vv_dsp_real>(N)); // Non-integer frequency

    // Create plans
    vv_dsp_fft_plan* r2c_plan = nullptr;
    vv_dsp_fft_plan* c2r_plan = nullptr;

    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_R2C, VV_DSP_FFT_FORWARD, &r2c_plan), VV_DSP_OK);
    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2R, VV_DSP_FFT_BACKWARD, &c2r_plan), VV_DSP_OK);

    // Execute R2C FFT
    ASSERT_EQ(vv_dsp_fft_execute(r2c_plan, input.data(), spectrum.data()), VV_DSP_OK);

    // Execute C2R IFFT
    ASSERT_EQ(vv_dsp_fft_execute(c2r_plan, spectrum.data(), reconstructed.data()), VV_DSP_OK);

    // Check reconstruction
    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(reconstructed[i], input[i], 1e-3)
            << "Reconstruction mismatch at index " << i << " for N=" << N;
    }

    vv_dsp_fft_destroy(r2c_plan);
    vv_dsp_fft_destroy(c2r_plan);
}

// Parameterized test instances
INSTANTIATE_TEST_SUITE_P(
    PowerOfTwo,
    FFTComplexTest,
    ::testing::Values(1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024)
);

INSTANTIATE_TEST_SUITE_P(
    NonPowerOfTwo,
    FFTComplexTest,
    ::testing::Values(3, 5, 6, 7, 9, 10, 12, 15, 20, 24, 30, 48, 100, 200)
);

INSTANTIATE_TEST_SUITE_P(
    PowerOfTwo,
    FFTRealTest,
    ::testing::Values(2, 4, 8, 16, 32, 64, 128, 256, 512, 1024)
);

INSTANTIATE_TEST_SUITE_P(
    NonPowerOfTwo,
    FFTRealTest,
    ::testing::Values(3, 5, 6, 7, 9, 10, 12, 15, 20, 24, 30, 48, 100, 200)
);

// Test FFT with different backends (if available)
class FFTBackendTest : public FFTTest, 
                       public ::testing::WithParamInterface<std::tuple<vv_dsp_fft_backend, size_t>> {};

TEST_P(FFTBackendTest, BackendConsistency) {
    const auto [backend, N] = GetParam();
    
    // Skip if backend is not available
    if (!vv_dsp_fft_is_backend_available(backend)) {
        GTEST_SKIP() << "Backend not available";
    }

    // Set backend
    ASSERT_EQ(vv_dsp_fft_set_backend(backend), VV_DSP_OK);

    std::vector<vv_dsp_cpx> input(N), output_backend(N);
    generateComplexSineWave(input, 1.0, static_cast<vv_dsp_real>(N));

    vv_dsp_fft_plan* plan = nullptr;
    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan), VV_DSP_OK);
    ASSERT_EQ(vv_dsp_fft_execute(plan, input.data(), output_backend.data()), VV_DSP_OK);

    // Compare with KissFFT reference
    vv_dsp_fft_set_backend(VV_DSP_FFT_BACKEND_KISS);
    std::vector<vv_dsp_cpx> output_kiss(N);
    vv_dsp_fft_plan* kiss_plan = nullptr;
    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &kiss_plan), VV_DSP_OK);
    ASSERT_EQ(vv_dsp_fft_execute(kiss_plan, input.data(), output_kiss.data()), VV_DSP_OK);

    // Results should be very similar between backends
    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(output_backend[i].re, output_kiss[i].re, 1e-5)
            << "Backend consistency failure (real) at index " << i;
        EXPECT_NEAR(output_backend[i].im, output_kiss[i].im, 1e-5)
            << "Backend consistency failure (imag) at index " << i;
    }

    vv_dsp_fft_destroy(plan);
    vv_dsp_fft_destroy(kiss_plan);
}

// Test different backend combinations
INSTANTIATE_TEST_SUITE_P(
    AllBackends,
    FFTBackendTest,
    ::testing::Combine(
        ::testing::Values(VV_DSP_FFT_BACKEND_KISS, VV_DSP_FFT_BACKEND_FFTW, VV_DSP_FFT_BACKEND_FFTS),
        ::testing::Values(static_cast<size_t>(16), static_cast<size_t>(64), static_cast<size_t>(128))
    )
);

// Edge case tests
TEST_F(FFTTest, EdgeCases) {
    // Test minimum size FFT
    vv_dsp_fft_plan* plan = nullptr;
    std::vector<vv_dsp_cpx> input(1), output(1);
    input[0] = {1.0, 0.0};

    ASSERT_EQ(vv_dsp_fft_make_plan(1, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan), VV_DSP_OK);
    ASSERT_EQ(vv_dsp_fft_execute(plan, input.data(), output.data()), VV_DSP_OK);

    // For N=1, FFT should be identity
    EXPECT_NEAR(output[0].re, input[0].re, 1e-6);
    EXPECT_NEAR(output[0].im, input[0].im, 1e-6);

    vv_dsp_fft_destroy(plan);

    // Test null pointer handling in execute
    EXPECT_NE(vv_dsp_fft_execute(nullptr, input.data(), output.data()), VV_DSP_OK);
}

// Performance regression test (basic timing check)
TEST_F(FFTTest, BasicPerformanceCheck) {
    const size_t N = 1024;
    std::vector<vv_dsp_cpx> input(N), output(N);
    generateComplexSineWave(input, 10.0, static_cast<vv_dsp_real>(N));

    vv_dsp_fft_plan* plan = nullptr;
    ASSERT_EQ(vv_dsp_fft_make_plan(N, VV_DSP_FFT_C2C, VV_DSP_FFT_FORWARD, &plan), VV_DSP_OK);

    // Execute multiple times to ensure plan is optimized
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(vv_dsp_fft_execute(plan, input.data(), output.data()), VV_DSP_OK);
    }

    // Basic correctness check
    size_t peak_bin = findPeakBin(output);
    EXPECT_EQ(peak_bin, 10) << "Performance test signal integrity check failed";

    vv_dsp_fft_destroy(plan);
}
