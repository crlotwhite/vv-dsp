/**
 * @file filter.h
 * @brief Digital filtering operations and filter design
 * @ingroup filter_group
 *
 * This is the main header for digital filtering functionality, providing access to
 * FIR filters, IIR filters, Savitzky-Golay smoothing filters, and common filtering
 * utilities. It serves as an umbrella header for all filtering operations.
 */

#ifndef VV_DSP_FILTER_H
#define VV_DSP_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/**
 * @defgroup filter_group Digital Filtering
 * @brief Digital filter design, implementation, and application
 *
 * The filter module provides comprehensive digital filtering capabilities including:
 * - **FIR Filters**: Finite Impulse Response filters with linear phase response
 * - **IIR Filters**: Infinite Impulse Response filters for efficient recursive filtering
 * - **Savitzky-Golay Filters**: Polynomial-based smoothing and differentiation filters
 * - **Filter Utilities**: Common operations like zero-phase filtering (filtfilt)
 *
 * All filters support both time-domain and frequency-domain implementations,
 * with automatic selection based on filter characteristics and input size.
 *
 * @{
 */

// Include all filtering submodules
#include "vv_dsp/filter/common.h"  ///< Common filtering utilities and helper functions
#include "vv_dsp/filter/fir.h"     ///< Finite Impulse Response (FIR) filters
#include "vv_dsp/filter/iir.h"     ///< Infinite Impulse Response (IIR) filters
#include "vv_dsp/filter/savgol.h"  ///< Savitzky-Golay smoothing and differentiation filters

/**
 * @brief Dummy function for basic filter module testing
 * @return Always returns 0
 * @note This function is primarily used for build system validation and sanity tests
 */
int vv_dsp_filter_dummy(void);

/** @} */ // End of filter_group

#ifdef __cplusplus
} // extern "C"
#endif

#endif // VV_DSP_FILTER_H
