/**
 * @file spectral.h
 * @brief Spectral analysis and frequency domain operations
 * @ingroup spectral_group
 *
 * This is the main header for spectral analysis functionality, including FFT, STFT,
 * DCT, CZT, and Hilbert transforms. It serves as an umbrella header that includes
 * all spectral analysis submodules.
 */

#ifndef VV_DSP_SPECTRAL_H
#define VV_DSP_SPECTRAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup spectral_group Spectral Analysis
 * @brief Frequency domain analysis and transforms
 *
 * The spectral module provides comprehensive frequency domain analysis tools including:
 * - Fast Fourier Transform (FFT) with multiple backends
 * - Short-Time Fourier Transform (STFT) for time-frequency analysis
 * - Discrete Cosine Transform (DCT) for compression and analysis
 * - Chirp Z-Transform (CZT) for arbitrary frequency resolution
 * - Hilbert Transform for analytic signal generation
 * - Spectral utilities for phase manipulation and frequency shifting
 *
 * All transforms support both single and double precision arithmetic based on
 * the VV_DSP_USE_DOUBLE compilation flag.
 *
 * @{
 */

/**
 * @brief Dummy function for basic spectral module testing
 * @return Always returns 0
 * @note This function is primarily used for build system validation
 */
int vv_dsp_spectral_dummy(void);

/** @} */ // End of spectral_group

// Include all spectral analysis submodules
#include "vv_dsp/spectral/fft.h"      ///< Fast Fourier Transform operations
#include "vv_dsp/spectral/utils.h"    ///< Spectral utilities (fftshift, ifftshift)
#include "vv_dsp/spectral/stft.h"     ///< Short-Time Fourier Transform
#include "vv_dsp/spectral/dct.h"      ///< Discrete Cosine Transform
#include "vv_dsp/spectral/czt.h"      ///< Chirp Z-Transform
#include "vv_dsp/spectral/hilbert.h"  ///< Hilbert Transform and analytic signals

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_SPECTRAL_H
