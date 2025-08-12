/**
 * @file iirfilter_example.cpp
 * @brief Simple example demonstrating the usage of vv::dsp::IIRFilter
 */

#include "vv_dsp/filter/IIRFilter.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

int main() {
    std::cout << "=== IIRFilter C++ Wrapper Example ===" << std::endl;

    try {
        // Example 1: Create a low-pass filter using factory function
        const double sampleRate = 48000.0;
        const double cutoffFreq = 1000.0;
        const double q = 0.707;

        auto lpf = vv::dsp::IIRFilter::createLowpass(sampleRate, cutoffFreq, q);
        std::cout << "✓ Created low-pass filter: " << lpf.getNumStages() << " stages" << std::endl;

        // Example 2: Process some test data
        std::vector<float> input = {1.0f, 0.5f, -0.3f, 0.8f, -0.2f, 0.1f, 0.0f, -0.1f};
        std::vector<float> output(input.size());

        std::cout << "Input:  ";
        for (const auto& val : input) {
            std::cout << std::fixed << std::setprecision(2) << val << " ";
        }
        std::cout << std::endl;

        lpf.process(input, output);

        std::cout << "Output: ";
        for (const auto& val : output) {
            std::cout << std::fixed << std::setprecision(2) << val << " ";
        }
        std::cout << std::endl;

        // Example 3: Test reset functionality
        std::vector<float> input2 = {1.0f, 0.0f, 0.0f, 0.0f};
        std::vector<float> output2(input2.size());

        lpf.process(input2, output2);
        std::cout << "Before reset: ";
        for (const auto& val : output2) {
            std::cout << std::fixed << std::setprecision(3) << val << " ";
        }
        std::cout << std::endl;

        lpf.reset();
        lpf.process(input2, output2);
        std::cout << "After reset:  ";
        for (const auto& val : output2) {
            std::cout << std::fixed << std::setprecision(3) << val << " ";
        }
        std::cout << std::endl;

        // Example 4: Create custom filter with manual coefficients
        vv_dsp_biquad custom_biquad;
        custom_biquad.b0 = 0.5f; custom_biquad.b1 = 0.0f; custom_biquad.b2 = 0.0f;
        custom_biquad.a1 = 0.0f; custom_biquad.a2 = 0.0f;
        custom_biquad.z1 = 0.0f; custom_biquad.z2 = 0.0f;

        std::vector<vv_dsp_biquad> coeffs = {custom_biquad};
        auto custom_filter = vv::dsp::IIRFilter(coeffs);

        std::vector<float> test_input = {2.0f, 4.0f, 6.0f};
        std::vector<float> test_output(test_input.size());

        custom_filter.process(test_input, test_output);

        std::cout << "Custom filter (0.5x gain):" << std::endl;
        std::cout << "Input:  ";
        for (const auto& val : test_input) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        std::cout << "Output: ";
        for (const auto& val : test_output) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        std::cout << "\n🎉 All examples completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
