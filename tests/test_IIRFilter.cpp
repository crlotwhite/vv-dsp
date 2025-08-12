/**
 * @file test_IIRFilter.cpp
 * @brief Comprehensive unit tests for vv::dsp::IIRFilter C++ wrapper
 */

#include "vv_dsp/filter/IIRFilter.hpp"
#include <vector>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <algorithm>

using namespace vv::dsp;

// Test helper functions
namespace {
    constexpr float EPSILON = 1e-6f;

    bool floatEqual(float a, float b, float epsilon = EPSILON) {
        return std::abs(a - b) < epsilon;
    }

    void assertFloatEqual(float expected, float actual, const char* message) {
        if (!floatEqual(expected, actual)) {
            std::cerr << "FAIL: " << message
                     << " - Expected: " << expected
                     << ", Actual: " << actual
                     << ", Diff: " << std::abs(expected - actual) << std::endl;
            assert(false);
        }
    }

    // Create a simple pass-through biquad (unity gain, no filtering)
    vv_dsp_biquad createPassthrough() {
        vv_dsp_biquad bq;
        bq.b0 = 1.0f; bq.b1 = 0.0f; bq.b2 = 0.0f;
        bq.a1 = 0.0f; bq.a2 = 0.0f;
        bq.z1 = 0.0f; bq.z2 = 0.0f;
        return bq;
    }

    // Create a simple gain biquad (scales input by gain factor)
    vv_dsp_biquad createGain(float gain) {
        vv_dsp_biquad bq;
        bq.b0 = gain; bq.b1 = 0.0f; bq.b2 = 0.0f;
        bq.a1 = 0.0f; bq.a2 = 0.0f;
        bq.z1 = 0.0f; bq.z2 = 0.0f;
        return bq;
    }
}

// Test 1: Constructor validation
void testConstructorValidation() {
    std::cout << "Testing constructor validation..." << std::endl;

    // Test 1a: Empty coefficients should throw
    std::vector<vv_dsp_biquad> empty_coeffs;
    try {
        IIRFilter filter(empty_coeffs);
        assert(false && "Should have thrown std::invalid_argument");
    } catch (const std::invalid_argument&) {
        // Expected
    }

    // Test 1b: Valid coefficients should construct successfully
    std::vector<vv_dsp_biquad> valid_coeffs = {createPassthrough()};
    try {
        IIRFilter filter(valid_coeffs);
        assert(filter.getNumStages() == 1);
        assert(!filter.empty());
    } catch (...) {
        assert(false && "Should not have thrown for valid coefficients");
    }

    std::cout << "✓ Constructor validation tests passed" << std::endl;
}

// Test 2: Move semantics
void testMoveSemantics() {
    std::cout << "Testing move semantics..." << std::endl;

    std::vector<vv_dsp_biquad> coeffs = {createGain(2.0f)};

    // Test move constructor
    IIRFilter filter1(coeffs);
    assert(filter1.getNumStages() == 1);

    IIRFilter filter2 = std::move(filter1);
    assert(filter2.getNumStages() == 1);

    // Test move assignment
    IIRFilter filter3(coeffs);
    filter3 = std::move(filter2);
    assert(filter3.getNumStages() == 1);

    std::cout << "✓ Move semantics tests passed" << std::endl;
}

// Test 3: Process method functionality
void testProcessMethod() {
    std::cout << "Testing process method..." << std::endl;

    // Test 3a: Pass-through filter
    std::vector<vv_dsp_biquad> passthrough_coeffs = {createPassthrough()};
    IIRFilter filter(passthrough_coeffs);

    std::vector<vv_dsp_real> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    std::vector<vv_dsp_real> output(input.size());

    filter.process(input, output);

    for (size_t i = 0; i < input.size(); ++i) {
        assertFloatEqual(input[i], output[i], "Pass-through filter output mismatch");
    }

    // Test 3b: Gain filter
    std::vector<vv_dsp_biquad> gain_coeffs = {createGain(2.0f)};
    IIRFilter gain_filter(gain_coeffs);

    gain_filter.process(input, output);

    for (size_t i = 0; i < input.size(); ++i) {
        assertFloatEqual(input[i] * 2.0f, output[i], "Gain filter output mismatch");
    }

    // Test 3c: In-place processing
    std::vector<vv_dsp_real> inplace_data = {1.0f, 2.0f, 3.0f, 4.0f};
    gain_filter.process(inplace_data, inplace_data);

    std::vector<vv_dsp_real> expected = {2.0f, 4.0f, 6.0f, 8.0f};
    for (size_t i = 0; i < inplace_data.size(); ++i) {
        assertFloatEqual(expected[i], inplace_data[i], "In-place processing mismatch");
    }

    std::cout << "✓ Process method tests passed" << std::endl;
}

// Test 4: Reset functionality
void testResetFunctionality() {
    std::cout << "Testing reset functionality..." << std::endl;

    // Create a simple delay biquad: y[n] = x[n-1] (b0=0, b1=1, others=0)
    vv_dsp_biquad delay_bq;
    delay_bq.b0 = 0.0f; delay_bq.b1 = 1.0f; delay_bq.b2 = 0.0f;
    delay_bq.a1 = 0.0f; delay_bq.a2 = 0.0f;
    delay_bq.z1 = 0.0f; delay_bq.z2 = 0.0f;

    std::vector<vv_dsp_biquad> delay_coeffs = {delay_bq};
    IIRFilter filter(delay_coeffs);

    // Process initial signal
    std::vector<vv_dsp_real> input1 = {1.0f, 2.0f, 3.0f};
    std::vector<vv_dsp_real> output1(input1.size());
    filter.process(input1, output1);

    // Expected: [0, 1, 2] (delayed by 1 sample)
    assertFloatEqual(0.0f, output1[0], "First delay output");
    assertFloatEqual(1.0f, output1[1], "Second delay output");
    assertFloatEqual(2.0f, output1[2], "Third delay output");

    // Reset and process the same signal again
    filter.reset();
    std::vector<vv_dsp_real> output2(input1.size());
    filter.process(input1, output2);

    // Output should be identical to first run
    for (size_t i = 0; i < input1.size(); ++i) {
        assertFloatEqual(output1[i], output2[i], "Reset functionality mismatch");
    }

    std::cout << "✓ Reset functionality tests passed" << std::endl;
}

// Test 5: Multi-stage processing
void testMultiStageProcessing() {
    std::cout << "Testing multi-stage processing..." << std::endl;

    // Create a two-stage filter: gain of 2, then gain of 3 = total gain of 6
    std::vector<vv_dsp_biquad> multi_coeffs = {
        createGain(2.0f),
        createGain(3.0f)
    };

    IIRFilter filter(multi_coeffs);
    assert(filter.getNumStages() == 2);

    std::vector<vv_dsp_real> input = {1.0f, 2.0f, 3.0f};
    std::vector<vv_dsp_real> output(input.size());

    filter.process(input, output);

    for (size_t i = 0; i < input.size(); ++i) {
        assertFloatEqual(input[i] * 6.0f, output[i], "Multi-stage processing mismatch");
    }

    std::cout << "✓ Multi-stage processing tests passed" << std::endl;
}

// Test 6: Factory functions
void testFactoryFunctions() {
    std::cout << "Testing factory functions..." << std::endl;

    const double sampleRate = 48000.0;
    const double frequency = 1000.0;
    const double q = 0.707;

    // Test 6a: createLowpass
    try {
        auto lpf = IIRFilter::createLowpass(sampleRate, frequency, q);
        assert(lpf.getNumStages() == 1);
        assert(!lpf.empty());
    } catch (...) {
        assert(false && "createLowpass should not throw for valid parameters");
    }

    // Test 6b: createHighpass
    try {
        auto hpf = IIRFilter::createHighpass(sampleRate, frequency, q);
        assert(hpf.getNumStages() == 1);
        assert(!hpf.empty());
    } catch (...) {
        assert(false && "createHighpass should not throw for valid parameters");
    }

    // Test 6c: createBandpass
    try {
        auto bpf = IIRFilter::createBandpass(sampleRate, frequency, 100.0);
        assert(bpf.getNumStages() == 1);
        assert(!bpf.empty());
    } catch (...) {
        assert(false && "createBandpass should not throw for valid parameters");
    }

    // Test 6d: Invalid parameters should throw
    try {
        auto invalid = IIRFilter::createLowpass(-1.0, frequency, q);
        assert(false && "Should have thrown for negative sample rate");
    } catch (const std::invalid_argument&) {
        // Expected
    }

    try {
        auto invalid = IIRFilter::createLowpass(sampleRate, sampleRate, q);
        assert(false && "Should have thrown for frequency >= Nyquist");
    } catch (const std::invalid_argument&) {
        // Expected
    }

    std::cout << "✓ Factory function tests passed" << std::endl;
}

// Test 7: Edge cases
void testEdgeCases() {
    std::cout << "Testing edge cases..." << std::endl;

    std::vector<vv_dsp_biquad> coeffs = {createPassthrough()};
    IIRFilter filter(coeffs);

    // Test 7a: Empty input/output
    std::vector<vv_dsp_real> empty_input;
    std::vector<vv_dsp_real> empty_output;
    filter.process(empty_input, empty_output);  // Should not crash

    // Test 7b: Single sample
    std::vector<vv_dsp_real> single_input = {42.0f};
    std::vector<vv_dsp_real> single_output(1);
    filter.process(single_input, single_output);
    assertFloatEqual(42.0f, single_output[0], "Single sample processing");

    std::cout << "✓ Edge case tests passed" << std::endl;
}

int main() {
    std::cout << "Running IIRFilter comprehensive unit tests..." << std::endl;

    try {
        testConstructorValidation();
        testMoveSemantics();
        testProcessMethod();
        testResetFunctionality();
        testMultiStageProcessing();
        testFactoryFunctions();
        testEdgeCases();

        std::cout << "\n🎉 All IIRFilter tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
