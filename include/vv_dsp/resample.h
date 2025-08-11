/**
 * @file resample.h
 * @brief Sample rate conversion and signal resampling
 * @ingroup resample_group
 *
 * This module provides comprehensive sample rate conversion capabilities including
 * interpolation, decimation, and arbitrary rate conversion with anti-aliasing filtering.
 */

#ifndef VV_DSP_RESAMPLE_H
#define VV_DSP_RESAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vv_dsp/vv_dsp_types.h"

/**
 * @defgroup resample_group Sample Rate Conversion
 * @brief Signal resampling and interpolation operations
 *
 * The resample module provides tools for changing the sample rate of digital signals
 * while preserving signal quality through proper anti-aliasing filtering.
 *
 * Features include:
 * - Arbitrary rate conversion (L/M ratios)
 * - High-quality anti-aliasing filters
 * - Memory-efficient streaming operation
 * - Support for both real and complex signals
 *
 * @{
 */

#include "vv_dsp/resample/interpolate.h"  ///< Interpolation algorithms
#include "vv_dsp/resample/resampler.h"    ///< Complete resampling systems

/**
 * @brief Dummy function for basic resample module testing
 * @return Always returns 0
 * @note This function is primarily used for build system validation
 */
int vv_dsp_resample_dummy(void);

/** @} */ // End of resample_group

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_RESAMPLE_H
