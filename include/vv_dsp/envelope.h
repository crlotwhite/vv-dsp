/**
 * @file envelope.h
 * @brief Signal envelope extraction and spectral envelope analysis
 * @ingroup envelope_group
 *
 * This module provides methods for extracting and analyzing signal envelopes,
 * including cepstral analysis, minimum-phase reconstruction, and linear
 * predictive coding (LPC) based spectral envelope estimation.
 */

#ifndef VV_DSP_ENVELOPE_H
#define VV_DSP_ENVELOPE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup envelope_group Envelope Analysis
 * @brief Signal and spectral envelope extraction techniques
 *
 * The envelope module provides various methods for analyzing signal envelopes:
 *
 * - **Cepstral Analysis**: Real cepstrum for envelope separation
 * - **Minimum-Phase Processing**: Signal reconstruction from envelope
 * - **Linear Predictive Coding (LPC)**: Parametric spectral envelope modeling
 *
 * These techniques are fundamental in speech processing, audio analysis,
 * and system identification applications.
 *
 * @{
 */

#include "vv_dsp/envelope/cepstrum.h"  ///< Real cepstrum analysis
#include "vv_dsp/envelope/minphase.h"  ///< Minimum-phase signal reconstruction
#include "vv_dsp/envelope/lpc.h"       ///< Linear Predictive Coding

/**
 * @brief Dummy function for basic envelope module testing
 * @return Always returns 0
 * @note This function is primarily used for build system validation and compatibility
 */
int vv_dsp_envelope_dummy(void);

/** @} */ // End of envelope_group

#ifdef __cplusplus
}
#endif

#endif // VV_DSP_ENVELOPE_H
