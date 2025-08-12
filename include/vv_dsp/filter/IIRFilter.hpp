/**
 * @file IIRFilter.hpp
 * @brief Modern C++ wrapper for IIR filtering functionality
 * @ingroup filter_group
 *
 * This file provides a modern, exception-safe C++ interface for IIR biquad
 * filtering with RAII resource management and std::span-based processing.
 */

#ifndef VV_DSP_FILTER_IIRFILTER_HPP
#define VV_DSP_FILTER_IIRFILTER_HPP

#include <vector>
#include <span>
#include <stdexcept>
#include <cassert>

#include "vv_dsp/filter/iir.h"
#include "vv_dsp/vv_dsp_types.h"

/**
 * @namespace vv::dsp
 * @brief Modern C++ DSP functionality namespace
 */
namespace vv {
namespace dsp {

/**
 * @brief Modern C++ wrapper for IIR biquad filtering
 * @ingroup filter_group
 *
 * This class provides a safe, modern C++ interface for IIR filtering operations.
 * It manages a chain of biquad sections and their state using RAII principles,
 * ensuring proper resource management and exception safety.
 *
 * Key features:
 * - RAII resource management (automatic cleanup)
 * - Move-only semantics (non-copyable, but movable)
 * - Real-time safe processing method (noexcept)
 * - std::span-based interface for modern C++
 * - Support for both in-place and out-of-place processing
 * - Denormal flushing for performance-critical real-time applications
 *
 * @code{.cpp}
 * // Create biquad coefficients
 * std::vector<vv_dsp_biquad> coeffs = createLowpassCoeffs(48000.0, 1000.0, 0.707);
 *
 * // Create filter
 * auto filter = vv::dsp::IIRFilter(coeffs);
 *
 * // Process audio
 * std::vector<float> input = {...};
 * std::vector<float> output(input.size());
 * filter.process(input, output);
 * @endcode
 */
class IIRFilter {
public:
    /**
     * @brief Construct an IIR filter with the given biquad coefficients
     * @param coeffs Span of biquad coefficient structures
     * @throws std::invalid_argument If coeffs is empty
     * @throws std::bad_alloc If memory allocation fails
     *
     * The constructor follows the strong exception safety guarantee - if it throws,
     * no resources are leaked and the program state is unchanged.
     */
    explicit IIRFilter(std::span<const vv_dsp_biquad> coeffs);

    /**
     * @brief Destructor - automatically cleans up all resources
     *
     * RAII ensures that all allocated memory is properly freed when the
     * IIRFilter object goes out of scope.
     */
    ~IIRFilter() = default;

    /**
     * @brief Copy constructor (deleted - move-only semantics)
     *
     * IIRFilter objects cannot be copied to prevent accidental duplication
     * of filter state and ensure clear ownership semantics.
     */
    IIRFilter(const IIRFilter&) = delete;

    /**
     * @brief Copy assignment operator (deleted - move-only semantics)
     */
    IIRFilter& operator=(const IIRFilter&) = delete;

    /**
     * @brief Move constructor (default implementation)
     *
     * Allows efficient transfer of filter ownership. The moved-from object
     * is left in a valid but unspecified state.
     */
    IIRFilter(IIRFilter&&) = default;

    /**
     * @brief Move assignment operator (default implementation)
     */
    IIRFilter& operator=(IIRFilter&&) = default;

    /**
     * @brief Process audio samples through the IIR filter chain
     * @param input Input audio samples (read-only)
     * @param output Output audio samples (write-only)
     * @throws None (noexcept for real-time safety)
     *
     * This method is real-time safe and will not allocate memory or throw exceptions.
     * Precondition checks are handled with assertions for performance.
     *
     * The method supports both in-place processing (when input.data() == output.data())
     * and out-of-place processing. Each sample is processed through the entire
     * biquad chain in series.
     *
     * @pre input.size() == output.size()
     * @pre input.data() != nullptr && output.data() != nullptr
     */
    void process(std::span<const vv_dsp_real> input, std::span<vv_dsp_real> output) noexcept;

    /**
     * @brief Reset the filter's internal state
     * @throws None (noexcept for real-time safety)
     *
     * Clears all delay line memory (z1, z2) in each biquad section, effectively
     * resetting the filter to its initial state. This is useful when starting
     * to process a new audio stream or after a discontinuity.
     */
    void reset() noexcept;

    /**
     * @brief Get the number of biquad stages in this filter
     * @return Number of biquad sections
     * @throws None (noexcept)
     */
    size_t getNumStages() const noexcept { return biquads_.size(); }

    /**
     * @brief Check if the filter is empty (no biquad stages)
     * @return True if the filter has no stages, false otherwise
     * @throws None (noexcept)
     */
    bool empty() const noexcept { return biquads_.empty(); }

    // Static factory functions for common filter types

    /**
     * @brief Create a low-pass IIR filter
     * @param sampleRate Sample rate in Hz
     * @param frequency Cutoff frequency in Hz
     * @param q Quality factor (typically 0.707 for Butterworth response)
     * @return IIRFilter configured as a low-pass filter
     * @throws std::invalid_argument If parameters are invalid
     * @throws std::bad_alloc If memory allocation fails
     *
     * Creates a second-order low-pass filter using the bilinear transform.
     */
    static IIRFilter createLowpass(double sampleRate, double frequency, double q = 0.707);

    /**
     * @brief Create a high-pass IIR filter
     * @param sampleRate Sample rate in Hz
     * @param frequency Cutoff frequency in Hz
     * @param q Quality factor (typically 0.707 for Butterworth response)
     * @return IIRFilter configured as a high-pass filter
     * @throws std::invalid_argument If parameters are invalid
     * @throws std::bad_alloc If memory allocation fails
     */
    static IIRFilter createHighpass(double sampleRate, double frequency, double q = 0.707);

    /**
     * @brief Create a band-pass IIR filter
     * @param sampleRate Sample rate in Hz
     * @param centerFreq Center frequency in Hz
     * @param bandwidth Bandwidth in Hz
     * @return IIRFilter configured as a band-pass filter
     * @throws std::invalid_argument If parameters are invalid
     * @throws std::bad_alloc If memory allocation fails
     */
    static IIRFilter createBandpass(double sampleRate, double centerFreq, double bandwidth);

private:
    std::vector<vv_dsp_biquad> biquads_;  ///< Biquad coefficients and state

    /**
     * @brief Validate biquad coefficients for stability and sanity
     * @param coeffs Coefficients to validate
     * @throws std::invalid_argument If coefficients are invalid
     */
    static void validateCoefficients(std::span<const vv_dsp_biquad> coeffs);

    /**
     * @brief Calculate biquad coefficients for a low-pass filter
     * @param sampleRate Sample rate in Hz
     * @param frequency Cutoff frequency in Hz
     * @param q Quality factor
     * @return Biquad coefficients structure
     */
    static vv_dsp_biquad calculateLowpassCoeffs(double sampleRate, double frequency, double q);

    /**
     * @brief Calculate biquad coefficients for a high-pass filter
     * @param sampleRate Sample rate in Hz
     * @param frequency Cutoff frequency in Hz
     * @param q Quality factor
     * @return Biquad coefficients structure
     */
    static vv_dsp_biquad calculateHighpassCoeffs(double sampleRate, double frequency, double q);

    /**
     * @brief Calculate biquad coefficients for a band-pass filter
     * @param sampleRate Sample rate in Hz
     * @param centerFreq Center frequency in Hz
     * @param bandwidth Bandwidth in Hz
     * @return Biquad coefficients structure
     */
    static vv_dsp_biquad calculateBandpassCoeffs(double sampleRate, double centerFreq, double bandwidth);
};

} // namespace dsp
} // namespace vv

#endif // VV_DSP_FILTER_IIRFILTER_HPP
