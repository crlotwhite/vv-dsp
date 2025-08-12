/**
 * @file IIRFilter.cpp
 * @brief Implementation of modern C++ wrapper for IIR filtering functionality
 */

#include "vv_dsp/filter/IIRFilter.hpp"
#include "vv_dsp/core/fp_env.h"  // For denormal handling from Task 25
#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>

namespace vv {
namespace dsp {

namespace {
    // Helper function to validate a single biquad coefficient set
    bool isValidBiquad(const vv_dsp_biquad& bq) {
        // Check for NaN or infinite values in coefficients
        if (!std::isfinite(bq.b0) || !std::isfinite(bq.b1) || !std::isfinite(bq.b2) ||
            !std::isfinite(bq.a1) || !std::isfinite(bq.a2)) {
            return false;
        }

        // Check for basic stability: poles should be inside unit circle
        // For a biquad with denominators a1, a2, the poles are roots of z^2 + a1*z + a2 = 0
        // Stability condition: |a2| < 1 and |a1| < 1 + a2
        const vv_dsp_real abs_a2 = std::abs(bq.a2);
        const vv_dsp_real abs_a1 = std::abs(bq.a1);

        if (abs_a2 >= static_cast<vv_dsp_real>(1.0)) {
            return false;  // Unstable: pole outside unit circle
        }

        if (abs_a1 >= static_cast<vv_dsp_real>(1.0) + abs_a2) {
            return false;  // Unstable: pole outside unit circle
        }

        return true;
    }
}

IIRFilter::IIRFilter(std::span<const vv_dsp_biquad> coeffs) {
    // Strong exception safety: validate all inputs before allocating
    if (coeffs.empty()) {
        throw std::invalid_argument("IIRFilter: coefficient span cannot be empty");
    }

    // Validate all coefficients for stability and finite values
    validateCoefficients(coeffs);

    // Reserve space to avoid multiple allocations
    biquads_.reserve(coeffs.size());

    // Copy coefficients and initialize state
    for (const auto& coeff : coeffs) {
        vv_dsp_biquad bq = coeff;  // Copy coefficients
        bq.z1 = static_cast<vv_dsp_real>(0.0);  // Initialize state
        bq.z2 = static_cast<vv_dsp_real>(0.0);
        biquads_.push_back(bq);
    }

    // If we reach here, construction was successful
    // The vector's destructor will handle cleanup if needed
}

void IIRFilter::process(std::span<const vv_dsp_real> input,
                       std::span<vv_dsp_real> output) noexcept {
    // Precondition checks with assertions (for performance in release builds)
    assert(input.size() == output.size());

    const size_t num_samples = input.size();
    const size_t num_stages = biquads_.size();

    // Handle edge cases
    if (num_samples == 0 || num_stages == 0) {
        return;
    }

    // Additional safety checks for non-empty spans
    assert(input.data() != nullptr);
    assert(output.data() != nullptr);

    // Process each sample through the biquad chain
    for (size_t i = 0; i < num_samples; ++i) {
        vv_dsp_real sample = input[i];

        // Apply each biquad stage in series
        for (size_t stage = 0; stage < num_stages; ++stage) {
            vv_dsp_biquad& bq = biquads_[stage];

            // Direct Form II Transposed implementation
            // (same as vv_dsp_biquad_process but inlined for performance)
            const vv_dsp_real y = bq.b0 * sample + bq.z1;
            bq.z1 = bq.b1 * sample - bq.a1 * y + bq.z2;
            bq.z2 = bq.b2 * sample - bq.a2 * y;

            sample = y;
        }

        output[i] = sample;
    }
}

void IIRFilter::reset() noexcept {
    // Clear all delay line memory
    for (auto& bq : biquads_) {
        bq.z1 = static_cast<vv_dsp_real>(0.0);
        bq.z2 = static_cast<vv_dsp_real>(0.0);
    }
}

void IIRFilter::validateCoefficients(std::span<const vv_dsp_biquad> coeffs) {
    for (size_t i = 0; i < coeffs.size(); ++i) {
        if (!isValidBiquad(coeffs[i])) {
            std::ostringstream oss;
            oss << "IIRFilter: invalid or unstable biquad coefficients at index " << i;
            throw std::invalid_argument(oss.str());
        }
    }
}

// Static factory function implementations

IIRFilter IIRFilter::createLowpass(double sampleRate, double frequency, double q) {
    if (sampleRate <= 0.0) {
        throw std::invalid_argument("IIRFilter::createLowpass: sample rate must be positive");
    }
    if (frequency <= 0.0 || frequency >= sampleRate * 0.5) {
        throw std::invalid_argument("IIRFilter::createLowpass: frequency must be positive and less than Nyquist");
    }
    if (q <= 0.0) {
        throw std::invalid_argument("IIRFilter::createLowpass: Q must be positive");
    }

    vv_dsp_biquad coeffs = calculateLowpassCoeffs(sampleRate, frequency, q);
    std::vector<vv_dsp_biquad> coeffVector = {coeffs};

    return IIRFilter(std::span<const vv_dsp_biquad>(coeffVector));
}

IIRFilter IIRFilter::createHighpass(double sampleRate, double frequency, double q) {
    if (sampleRate <= 0.0) {
        throw std::invalid_argument("IIRFilter::createHighpass: sample rate must be positive");
    }
    if (frequency <= 0.0 || frequency >= sampleRate * 0.5) {
        throw std::invalid_argument("IIRFilter::createHighpass: frequency must be positive and less than Nyquist");
    }
    if (q <= 0.0) {
        throw std::invalid_argument("IIRFilter::createHighpass: Q must be positive");
    }

    vv_dsp_biquad coeffs = calculateHighpassCoeffs(sampleRate, frequency, q);
    std::vector<vv_dsp_biquad> coeffVector = {coeffs};

    return IIRFilter(std::span<const vv_dsp_biquad>(coeffVector));
}

IIRFilter IIRFilter::createBandpass(double sampleRate, double centerFreq, double bandwidth) {
    if (sampleRate <= 0.0) {
        throw std::invalid_argument("IIRFilter::createBandpass: sample rate must be positive");
    }
    if (centerFreq <= 0.0 || centerFreq >= sampleRate * 0.5) {
        throw std::invalid_argument("IIRFilter::createBandpass: center frequency must be positive and less than Nyquist");
    }
    if (bandwidth <= 0.0 || bandwidth >= sampleRate * 0.5) {
        throw std::invalid_argument("IIRFilter::createBandpass: bandwidth must be positive and less than Nyquist");
    }

    vv_dsp_biquad coeffs = calculateBandpassCoeffs(sampleRate, centerFreq, bandwidth);
    std::vector<vv_dsp_biquad> coeffVector = {coeffs};

    return IIRFilter(std::span<const vv_dsp_biquad>(coeffVector));
}

// Coefficient calculation helpers

vv_dsp_biquad IIRFilter::calculateLowpassCoeffs(double sampleRate, double frequency, double q) {
    // Bilinear transform for 2nd order Butterworth low-pass
    const double omega = 2.0 * M_PI * frequency / sampleRate;
    const double sin_omega = std::sin(omega);
    const double cos_omega = std::cos(omega);
    const double alpha = sin_omega / (2.0 * q);

    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cos_omega;
    const double a2 = 1.0 - alpha;

    const double b0 = (1.0 - cos_omega) * 0.5;
    const double b1 = 1.0 - cos_omega;
    const double b2 = (1.0 - cos_omega) * 0.5;

    vv_dsp_biquad result;
    // Normalize by a0
    result.b0 = static_cast<vv_dsp_real>(b0 / a0);
    result.b1 = static_cast<vv_dsp_real>(b1 / a0);
    result.b2 = static_cast<vv_dsp_real>(b2 / a0);
    result.a1 = static_cast<vv_dsp_real>(a1 / a0);
    result.a2 = static_cast<vv_dsp_real>(a2 / a0);
    result.z1 = static_cast<vv_dsp_real>(0.0);
    result.z2 = static_cast<vv_dsp_real>(0.0);

    return result;
}

vv_dsp_biquad IIRFilter::calculateHighpassCoeffs(double sampleRate, double frequency, double q) {
    // Bilinear transform for 2nd order Butterworth high-pass
    const double omega = 2.0 * M_PI * frequency / sampleRate;
    const double sin_omega = std::sin(omega);
    const double cos_omega = std::cos(omega);
    const double alpha = sin_omega / (2.0 * q);

    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cos_omega;
    const double a2 = 1.0 - alpha;

    const double b0 = (1.0 + cos_omega) * 0.5;
    const double b1 = -(1.0 + cos_omega);
    const double b2 = (1.0 + cos_omega) * 0.5;

    vv_dsp_biquad result;
    // Normalize by a0
    result.b0 = static_cast<vv_dsp_real>(b0 / a0);
    result.b1 = static_cast<vv_dsp_real>(b1 / a0);
    result.b2 = static_cast<vv_dsp_real>(b2 / a0);
    result.a1 = static_cast<vv_dsp_real>(a1 / a0);
    result.a2 = static_cast<vv_dsp_real>(a2 / a0);
    result.z1 = static_cast<vv_dsp_real>(0.0);
    result.z2 = static_cast<vv_dsp_real>(0.0);

    return result;
}

vv_dsp_biquad IIRFilter::calculateBandpassCoeffs(double sampleRate, double centerFreq, double bandwidth) {
    // Bilinear transform for 2nd order band-pass
    const double omega = 2.0 * M_PI * centerFreq / sampleRate;
    const double sin_omega = std::sin(omega);
    const double cos_omega = std::cos(omega);
    const double q = centerFreq / bandwidth;  // Q from center freq and bandwidth
    const double alpha = sin_omega / (2.0 * q);

    const double a0 = 1.0 + alpha;
    const double a1 = -2.0 * cos_omega;
    const double a2 = 1.0 - alpha;

    const double b0 = alpha;
    const double b1 = 0.0;
    const double b2 = -alpha;

    vv_dsp_biquad result;
    // Normalize by a0
    result.b0 = static_cast<vv_dsp_real>(b0 / a0);
    result.b1 = static_cast<vv_dsp_real>(b1 / a0);
    result.b2 = static_cast<vv_dsp_real>(b2 / a0);
    result.a1 = static_cast<vv_dsp_real>(a1 / a0);
    result.a2 = static_cast<vv_dsp_real>(a2 / a0);
    result.z1 = static_cast<vv_dsp_real>(0.0);
    result.z2 = static_cast<vv_dsp_real>(0.0);

    return result;
}

} // namespace dsp
} // namespace vv
