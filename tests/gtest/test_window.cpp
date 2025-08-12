/**
 * @file test_window.cpp
 * @brief Google Test suite for vv-dsp window functions
 */

#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <algorithm>

extern "C" {
#include "vv_dsp/window.h"
#include "vv_dsp/vv_dsp_types.h"
}

class WindowTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common test setup
    }

    void TearDown() override {
        // Common test cleanup
    }

    // Helper function to check if all values are between 0 and 1
    bool isNormalized(const std::vector<vv_dsp_real>& window) {
        return std::all_of(window.begin(), window.end(),
                          [](vv_dsp_real val) { return val >= 0.0 && val <= 1.0; });
    }

    // Helper function to check symmetry
    bool isSymmetric(const std::vector<vv_dsp_real>& window, double tolerance = 1e-6) {
        size_t N = window.size();
        for (size_t i = 0; i < N/2; ++i) {
            if (std::abs(window[i] - window[N-1-i]) > tolerance) {
                return false;
            }
        }
        return true;
    }

    // Helper function to find maximum value
    vv_dsp_real findMax(const std::vector<vv_dsp_real>& window) {
        return *std::max_element(window.begin(), window.end());
    }
};

// Test Hann window basic properties
TEST_F(WindowTest, HannWindowBasicProperties) {
    const size_t N = 64;
    std::vector<vv_dsp_real> window(N);

    vv_dsp_status status = vv_dsp_window_hann(N, window.data());
    EXPECT_EQ(status, VV_DSP_OK);

    // Check that window is normalized (values between 0 and 1)
    EXPECT_TRUE(isNormalized(window));

    // Check that window is symmetric
    EXPECT_TRUE(isSymmetric(window));

    // Check that maximum value is approximately 1.0 (at center)
    EXPECT_NEAR(findMax(window), 1.0, 1e-3);

    // Check edge values (should be close to 0 for Hann)
    EXPECT_NEAR(window[0], 0.0, 1e-6);
    EXPECT_NEAR(window[N-1], 0.0, 1e-6);
}

// Test Hamming window basic properties
TEST_F(WindowTest, HammingWindowBasicProperties) {
    const size_t N = 64;
    std::vector<vv_dsp_real> window(N);

    vv_dsp_status status = vv_dsp_window_hamming(N, window.data());
    EXPECT_EQ(status, VV_DSP_OK);

    // Check that window is normalized
    EXPECT_TRUE(isNormalized(window));

    // Check that window is symmetric
    EXPECT_TRUE(isSymmetric(window));

    // Check that maximum value is approximately 1.0
    EXPECT_NEAR(findMax(window), 1.0, 1e-3);

    // Check edge values (should be around 0.08 for Hamming)
    EXPECT_NEAR(window[0], 0.08, 1e-2);
    EXPECT_NEAR(window[N-1], 0.08, 1e-2);
}

// Test Blackman window basic properties
TEST_F(WindowTest, BlackmanWindowBasicProperties) {
    const size_t N = 64;
    std::vector<vv_dsp_real> window(N);

    vv_dsp_status status = vv_dsp_window_blackman(N, window.data());
    EXPECT_EQ(status, VV_DSP_OK);

    // Check that all values are reasonable (between 0 and 1, with small tolerance for floating point errors)
    for (vv_dsp_real val : window) {
        EXPECT_GE(val, -1e-6f);  // Allow small negative values due to floating point precision
        EXPECT_LE(val, 1.0f);
    }

    // Check that window is symmetric
    EXPECT_TRUE(isSymmetric(window));

    // Check that maximum value is close to 1.0 (Blackman may not reach exactly 1.0)
    EXPECT_NEAR(findMax(window), 1.0, 5e-3);
}

// Test rectangular (boxcar) window
TEST_F(WindowTest, BoxcarWindowProperties) {
    const size_t N = 64;
    std::vector<vv_dsp_real> window(N);

    vv_dsp_status status = vv_dsp_window_boxcar(N, window.data());
    EXPECT_EQ(status, VV_DSP_OK);

    // All values should be 1.0 for boxcar window
    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(window[i], 1.0, 1e-10);
    }
}

// Test window with different sizes
class WindowSizeTest : public WindowTest, public ::testing::WithParamInterface<size_t> {};

TEST_P(WindowSizeTest, HannWindowDifferentSizes) {
    const size_t N = GetParam();
    std::vector<vv_dsp_real> window(N);

    vv_dsp_status status = vv_dsp_window_hann(N, window.data());
    EXPECT_EQ(status, VV_DSP_OK);

    if (N > 2) {  // Skip max value test for N=2 since Hann window maximum is 0 for N=2
        EXPECT_TRUE(isSymmetric(window));
        // Allow much larger tolerance for small windows since discrete sampling significantly affects behavior
        double tolerance = (N <= 4) ? 0.5 : (N <= 32) ? 5e-2 : 1e-3;
        EXPECT_NEAR(findMax(window), 1.0, tolerance);
    }
}

// Test various window sizes
INSTANTIATE_TEST_SUITE_P(
    VariousSizes,
    WindowSizeTest,
    ::testing::Values(1, 2, 4, 8, 16, 32, 64, 128, 512, 1024)
);

// Test Hann window specific values against known reference
TEST_F(WindowTest, HannWindowSpecificValues) {
    const size_t N = 8;
    std::vector<vv_dsp_real> window(N);

    vv_dsp_status status = vv_dsp_window_hann(N, window.data());
    EXPECT_EQ(status, VV_DSP_OK);

    // Expected values for N=8 Hann window (computed analytically)
    // w[n] = 0.5 - 0.5*cos(2*pi*n/(N-1))
    std::vector<vv_dsp_real> expected = {
        0.0,                           // n=0
        0.5 - 0.5*std::cos(2*M_PI*1/7), // n=1
        0.5 - 0.5*std::cos(2*M_PI*2/7), // n=2
        0.5 - 0.5*std::cos(2*M_PI*3/7), // n=3
        0.5 - 0.5*std::cos(2*M_PI*4/7), // n=4
        0.5 - 0.5*std::cos(2*M_PI*5/7), // n=5
        0.5 - 0.5*std::cos(2*M_PI*6/7), // n=6
        0.0                            // n=7
    };

    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(window[i], expected[i], 1e-6)
            << "Mismatch at index " << i;
    }
}

// Test Hamming window specific values against known reference
TEST_F(WindowTest, HammingWindowSpecificValues) {
    const size_t N = 8;
    std::vector<vv_dsp_real> window(N);

    vv_dsp_status status = vv_dsp_window_hamming(N, window.data());
    EXPECT_EQ(status, VV_DSP_OK);

    // Expected values for N=8 Hamming window
    // w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1))
    std::vector<vv_dsp_real> expected = {
        0.54 - 0.46*std::cos(2*M_PI*0/7), // n=0
        0.54 - 0.46*std::cos(2*M_PI*1/7), // n=1
        0.54 - 0.46*std::cos(2*M_PI*2/7), // n=2
        0.54 - 0.46*std::cos(2*M_PI*3/7), // n=3
        0.54 - 0.46*std::cos(2*M_PI*4/7), // n=4
        0.54 - 0.46*std::cos(2*M_PI*5/7), // n=5
        0.54 - 0.46*std::cos(2*M_PI*6/7), // n=6
        0.54 - 0.46*std::cos(2*M_PI*7/7)  // n=7
    };

    for (size_t i = 0; i < N; ++i) {
        EXPECT_NEAR(window[i], expected[i], 1e-6)
            << "Mismatch at index " << i;
    }
}

// Test edge cases
TEST_F(WindowTest, EdgeCases) {
    std::vector<vv_dsp_real> window(1);

    // Test with N=1
    vv_dsp_status status = vv_dsp_window_hann(1, window.data());
    EXPECT_EQ(status, VV_DSP_OK);
    EXPECT_NEAR(window[0], 1.0, 1e-10);

    // Test with null pointer (should fail gracefully)
    status = vv_dsp_window_hann(64, nullptr);
    EXPECT_NE(status, VV_DSP_OK);
}

// Test all window types for basic functionality
class AllWindowTypesTest : public WindowTest {};

TEST_F(AllWindowTypesTest, AllWindowTypesBasicFunctionality) {
    const size_t N = 64;
    std::vector<vv_dsp_real> window(N);

    // Test all window functions
    EXPECT_EQ(vv_dsp_window_boxcar(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_hann(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_hamming(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_blackman(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_blackman_harris(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_nuttall(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_bartlett(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_bohman(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_cosine(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_planck_taper(N, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_flattop(N, window.data()), VV_DSP_OK);

    // Test parametric windows with default parameters
    EXPECT_EQ(vv_dsp_window_kaiser(N, 8.0, window.data()), VV_DSP_OK);
    EXPECT_EQ(vv_dsp_window_tukey(N, 0.5, window.data()), VV_DSP_OK);
}
